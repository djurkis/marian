Marian
======

[![Build Status](https://img.shields.io/jenkins/s/http/vali.inf.ed.ac.uk/jenkins/view/marian/job/marian-dev.svg?label=CUDA)](http://vali.inf.ed.ac.uk/jenkins/job/marian-dev/)
[![CPU Build Status](https://img.shields.io/jenkins/s/http/vali.inf.ed.ac.uk/jenkins/view/marian/job/marian-dev-cpu.svg?label=CPU)](http://vali.inf.ed.ac.uk/jenkins/job/marian-dev-cpu/)
[![Tests Status](https://img.shields.io/jenkins/s/http/vali.inf.ed.ac.uk/jenkins/view/marian/job/marian-regression-tests.svg?label=tests)](http://vali.inf.ed.ac.uk/jenkins/job/marian-regression-tests/)
[![Latest release](https://img.shields.io/github/release/marian-nmt/marian.svg?label=release)](https://github.com/marian-nmt/marian/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](./LICENSE.md)
[![Twitter](https://img.shields.io/twitter/follow/marian_nmt.svg?style=social)](https://twitter.com/intent/follow?screen_name=marian_nmt)

 <p>
  <b>Marian</b> is an efficient Neural Machine Translation framework written
  in pure C++ with minimal dependencies. 

  Named in honour of Marian Rejewski, a Polish mathematician and cryptologist.

  <!--It has mainly been developed at the
  Adam Mickiewicz University in Poznań (AMU) and at the University of Edinburgh.-->
  </p>

  <!--p>
  It is currently being deployed in
  multiple European projects and is the main translation and training engine
  behind the neural MT launch at the
  <a href="http://www.wipo.int/pressroom/en/articles/2016/article_0014.html">World Intellectual Property Organization</a>.
  </p-->

  <p>
  Main features:
  <ul>
    <li> Fast multi-gpu training and translation </li>
    <li> Compatible with Nematus and DL4MT </li>
    <li> Efficient pure C++ implementation </li>
    <li> Permissive open source license (MIT) </li>
    <li> <a href="https://marian-nmt.github.io/features/"> more details... </a> </li>
  </ul>
  </p>

If you use this, please cite:

Marcin Junczys-Dowmunt, Roman Grundkiewicz, Tomasz Dwojak, Hieu Hoang, Kenneth
Heafield, Tom Neckermann, Frank Seide, Ulrich Germann, Alham Fikri Aji, Nikolay
Bogoychev, André F. T. Martins, Alexandra Birch (2018). Marian: Fast Neural
Machine Translation in C++ (http://www.aclweb.org/anthology/P18-4020)

    @InProceedings{mariannmt,
        title     = {Marian: Fast Neural Machine Translation in {C++}},
        author    = {Junczys-Dowmunt, Marcin and Grundkiewicz, Roman and
                     Dwojak, Tomasz and Hoang, Hieu and Heafield, Kenneth and
                     Neckermann, Tom and Seide, Frank and Germann, Ulrich and
                     Fikri Aji, Alham and Bogoychev, Nikolay and
                     Martins, Andr\'{e} F. T. and Birch, Alexandra},
        booktitle = {Proceedings of ACL 2018, System Demonstrations},
        pages     = {116--121},
        publisher = {Association for Computational Linguistics},
        year      = {2018},
        month     = {July},
        address   = {Melbourne, Australia},
        url       = {http://www.aclweb.org/anthology/P18-4020}
    }

<!--
## Compilation

```
cd marian-dev
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
```
-->

## Amun
The handwritten decoder for RNN models compatible with Marian and Nematus has been superseded by the Marian decoder. The code is available in a separate repository: https://github.com/marian-nmt/amun

## Website

More information on https://marian-nmt.github.io

- [Quick start](https://marian-nmt.github.io/quickstart)
- [Installation and usage documentation](https://marian-nmt.github.io/docs)
- [Usage examples](https://marian-nmt.github.io/examples)

## Acknowledgements

The development of Marian received funding from the European Union's
_Horizon 2020 Research and Innovation Programme_ under grant agreements
688139 ([SUMMA](http://www.summa-project.eu); 2016-2019),
645487 ([Modern MT](http://www.modernmt.eu); 2015-2017),
644333 ([TraMOOC](http://tramooc.eu/); 2015-2017),
644402 ([HiML](http://www.himl.eu/); 2015-2017),
825303 ([Bergamot](https://browser.mt/); 2019-2021),
the Amazon Academic Research Awards program,
the World Intellectual Property Organization,
and is based upon work supported in part by the Office of the Director of
National Intelligence (ODNI), Intelligence Advanced Research Projects Activity
(IARPA), via contract #FA8650-17-C-9117.

This software contains source code provided by NVIDIA Corporation.
