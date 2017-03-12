#pragma once

#include "models/encdec.h"
#include "layers/attention.h"

namespace marian {

typedef AttentionCell<GRU, GlobalAttention, GRU> CGRU;

class EncoderDL4MT : public EncoderBase {
  public:

    template <class ...Args>
    EncoderDL4MT(Ptr<Config> options, Args... args)
     : EncoderBase(options, args...) {}

    Ptr<EncoderState>
    build(Ptr<ExpressionGraph> graph,
          Ptr<data::CorpusBatch> batch,
          size_t batchIdx = 0) {

      using namespace keywords;

      int dimSrcVoc = options_->get<std::vector<int>>("dim-vocabs")[batchIdx];
      int dimSrcEmb = options_->get<int>("dim-emb");
      int dimEncState = options_->get<int>("dim-rnn");
      bool layerNorm = options_->get<bool>("normalize");

      float dropoutRnn = options_->get<float>("dropout-rnn");
      float dropoutSrc = options_->get<float>("dropout-src");

      auto xEmb = Embedding("Wemb", dimSrcVoc, dimSrcEmb)(graph);

      Expr x, xMask;
      std::tie(x, xMask) = prepareSource(xEmb, batch, batchIdx);

      if(dropoutSrc) {
        int dimBatch = x->shape()[0];
        int srcWords = x->shape()[2];
        auto srcWordDrop = graph->dropout(dropoutSrc, {dimBatch, 1, srcWords});
        x = dropout(x, mask=srcWordDrop);
      }

      auto xFw = RNN<GRU>(graph, prefix_,
                          dimSrcEmb, dimEncState,
                          normalize=layerNorm,
                          dropout_prob=dropoutRnn)
                         (x);

      auto xBw = RNN<GRU>(graph, prefix_ + "_r",
                          dimSrcEmb, dimEncState,
                          normalize=layerNorm,
                          direction=dir::backward,
                          dropout_prob=dropoutRnn)
                         (x, mask=xMask);

      auto xContext = concatenate({xFw, xBw}, axis=1);
      return New<EncoderState>(EncoderState{xContext, xMask});
    }
};

class DecoderDL4MT : public DecoderBase {
  private:
    Ptr<GlobalAttention> attention_;

  public:
    DecoderDL4MT(Ptr<Config> options)
     : DecoderBase(options) {}

    virtual std::tuple<Expr, std::vector<Expr>>
    step(Expr embeddings,
         std::vector<Expr> states,
         Ptr<EncoderState> encState,
         bool single) {
      using namespace keywords;

      int dimTrgVoc = options_->get<std::vector<int>>("dim-vocabs").back();
      int dimTrgEmb = options_->get<int>("dim-emb");
      int dimDecState = options_->get<int>("dim-rnn");
      bool layerNorm = options_->get<bool>("normalize");
      bool skipDepth = options_->get<bool>("skip");
      size_t decoderLayers = options_->get<size_t>("layers-dec");
      float dropoutRnn = options_->get<float>("dropout-rnn");
      float dropoutTrg = options_->get<float>("dropout-trg");

      auto graph = embeddings->graph();

      if(dropoutTrg) {
        int dimBatch = embeddings->shape()[0];
        int trgWords = embeddings->shape()[2];
        auto trgWordDrop = graph->dropout(dropoutTrg, {dimBatch, 1, trgWords});
        embeddings = dropout(embeddings, mask=trgWordDrop);
      }

      if(!attention_)
        attention_ = New<GlobalAttention>("decoder",
                                          encState,
                                          dimDecState,
                                          normalize=layerNorm);
      RNN<CGRU> rnnL1(graph, "decoder",
                      dimTrgEmb, dimDecState,
                      attention_,
                      normalize=layerNorm,
                      dropout_prob=dropoutRnn
                      );
      auto stateL1 = rnnL1(embeddings, states[0]);
      auto alignedContext = single ?
        rnnL1.getCell()->getLastContext() :
        rnnL1.getCell()->getContexts();

      std::vector<Expr> statesOut;
      statesOut.push_back(stateL1);

      Expr outputLn = stateL1;

      //// 2-layer feedforward network for outputs and cost
      auto logitsL1 = Dense("ff_logit_l1", dimTrgEmb,
                            activation=act::tanh,
                            normalize=layerNorm)
                        (embeddings, outputLn, alignedContext);

      auto logitsL2 = Dense("ff_logit_l2", dimTrgVoc)
                        (logitsL1);

      return std::make_tuple(logitsL2, statesOut);
    }

};

class DL4MT : public Seq2Seq<EncoderDL4MT, DecoderDL4MT> {
  public:
    DL4MT(Ptr<Config> options) : Seq2Seq(options) {}

    void load(Ptr<ExpressionGraph> graph,
              const std::string& name) {
      using namespace keywords;

      LOG(info, "Loading model from {}", name);

      auto numpy = cnpy::npz_load(name);

      std::vector<std::string> parameters = {
        // Source word embeddings
        "Wemb",

        // GRU in encoder
        "encoder_U", "encoder_W", "encoder_b",
        "encoder_Ux", "encoder_Wx", "encoder_bx",

        // GRU in encoder, reversed
        "encoder_r_U", "encoder_r_W", "encoder_r_b",
        "encoder_r_Ux", "encoder_r_Wx", "encoder_r_bx",

        // Transformation of decoder input state
        "ff_state_W", "ff_state_b",

        // Target word embeddings
        "Wemb_dec",

        // GRU layer 1 in decoder
        "decoder_U", "decoder_W", "decoder_b",
        "decoder_Ux", "decoder_Wx", "decoder_bx",

        // Attention
        "decoder_W_comb_att", "decoder_b_att",
        "decoder_Wc_att", "decoder_U_att",

        // GRU layer 2 in decoder
        "decoder_U_nl", "decoder_Wc", "decoder_b_nl",
        "decoder_Ux_nl", "decoder_Wcx", "decoder_bx_nl",

        // Read out
        "ff_logit_lstm_W", "ff_logit_lstm_b",
        "ff_logit_prev_W", "ff_logit_prev_b",
        "ff_logit_ctx_W", "ff_logit_ctx_b",
        "ff_logit_W", "ff_logit_b",
      };

      std::vector<std::string> parametersNorm = {
        "decoder_att_gamma1", "decoder_att_gamma2",
        "decoder_cell1_gamma1", "decoder_cell1_gamma2",
        "decoder_cell2_gamma1", "decoder_cell2_gamma2",
        "encoder_gamma1", "encoder_gamma2",
        "encoder_r_gamma1", "encoder_r_gamma2",
        "ff_logit_l1_gamma0", "ff_logit_l1_gamma1",
        "ff_logit_l1_gamma2", "ff_state_gamma"
      };

      if(options_->get<bool>("normalize"))
        for(auto& p : parametersNorm)
          parameters.push_back(p);

      std::map<std::string, std::string> nameMap = {
        {"decoder_U", "decoder_cell1_U"},
        {"decoder_W", "decoder_cell1_W"},
        {"decoder_b", "decoder_cell1_b"},
        {"decoder_Ux", "decoder_cell1_Ux"},
        {"decoder_Wx", "decoder_cell1_Wx"},
        {"decoder_bx", "decoder_cell1_bx"},

        {"decoder_U_nl", "decoder_cell2_U"},
        {"decoder_Wc", "decoder_cell2_W"},
        {"decoder_b_nl", "decoder_cell2_b"},
        {"decoder_Ux_nl", "decoder_cell2_Ux"},
        {"decoder_Wcx", "decoder_cell2_Wx"},
        {"decoder_bx_nl", "decoder_cell2_bx"},

        {"ff_logit_prev_W", "ff_logit_l1_W0"},
        {"ff_logit_prev_b", "ff_logit_l1_b0"},
        {"ff_logit_lstm_W", "ff_logit_l1_W1"},
        {"ff_logit_lstm_b", "ff_logit_l1_b1"},
        {"ff_logit_ctx_W", "ff_logit_l1_W2"},
        {"ff_logit_ctx_b", "ff_logit_l1_b2"},

        {"ff_logit_W", "ff_logit_l2_W"},
        {"ff_logit_b", "ff_logit_l2_b"}
      };

      for(auto name : parameters) {
        UTIL_THROW_IF2(numpy.count(name) == 0,
                       "Parameter " << name << " does not exist.");

        Shape shape;
        if(numpy[name].shape.size() == 2) {
          shape.set(0, numpy[name].shape[0]);
          shape.set(1, numpy[name].shape[1]);
        }
        else if(numpy[name].shape.size() == 1) {
          shape.set(0, 1);
          shape.set(1, numpy[name].shape[0]);
        }

        std::string pName = name;
        if(nameMap.count(name))
          pName = nameMap[name];

        graph->param(pName, shape,
                     init=inits::from_numpy(numpy[name]));
      }
    }

    void save(Ptr<ExpressionGraph> graph,
              const std::string& name) {

      LOG(info, "Saving model to {}", name);

      unsigned shape[2];
      std::string mode = "w";

      std::map<std::string, std::string> nameMap = {
        {"decoder_cell1_U", "decoder_U"},
        {"decoder_cell1_W", "decoder_W"},
        {"decoder_cell1_b", "decoder_b"},
        {"decoder_cell1_Ux", "decoder_Ux"},
        {"decoder_cell1_Wx", "decoder_Wx"},
        {"decoder_cell1_bx", "decoder_bx"},

        {"decoder_cell2_U", "decoder_U_nl"},
        {"decoder_cell2_W", "decoder_Wc"},
        {"decoder_cell2_b", "decoder_b_nl"},
        {"decoder_cell2_Ux", "decoder_Ux_nl"},
        {"decoder_cell2_Wx", "decoder_Wcx"},
        {"decoder_cell2_bx", "decoder_bx_nl"},

        {"ff_logit_l1_W0", "ff_logit_prev_W"},
        {"ff_logit_l1_b0", "ff_logit_prev_b"},
        {"ff_logit_l1_W1", "ff_logit_lstm_W"},
        {"ff_logit_l1_b1", "ff_logit_lstm_b"},
        {"ff_logit_l1_W2", "ff_logit_ctx_W"},
        {"ff_logit_l1_b2", "ff_logit_ctx_b"},

        {"ff_logit_l2_W", "ff_logit_W"},
        {"ff_logit_l2_b", "ff_logit_b"}
      };

      cudaSetDevice(graph->getDevice());

      for(auto p : graph->params().getMap()) {
        std::vector<float> v;
        p.second->val() >> v;

        unsigned dim;
        if(p.second->shape()[0] == 1) {
          shape[0] = p.second->shape()[1];
          dim = 1;
        }
        else {
          shape[0] = p.second->shape()[0];
          shape[1] = p.second->shape()[1];
          dim = 2;
        }

        std::string pName = p.first;
        if(nameMap.count(pName))
          pName = nameMap[pName];

        cnpy::npz_save(name, pName, v.data(), shape, dim, mode);
        mode = "a";
      }

      float ctt = 0;
      shape[0] = 1;
      cnpy::npz_save(name, "decoder_c_tt", &ctt, shape, 1, mode);
    }
};

}