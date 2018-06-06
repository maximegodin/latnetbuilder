// This file is part of LatNet Builder.
//
// Copyright (C) 2012-2018  Pierre L'Ecuyer and Universite de Montreal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef LATBUILDER__TASK__RANDOM_H
#define LATBUILDER__TASK__RANDOM_H

#include "latbuilder/Task/LatSeqBasedSearch.h"
#include "latbuilder/Task/macros.h"

#include "latbuilder/MeritSeq/CBC.h"
#include "latbuilder/LatSeq/Combiner.h"
#include "latbuilder/GenSeq/GeneratingValues.h"
#include "latbuilder/GenSeq/VectorCreator.h"
#include "latbuilder/SizeParam.h"
#include "latbuilder/Traversal.h"
#include "latbuilder/LFSR113.h"

#include <boost/lexical_cast.hpp>

namespace LatBuilder { namespace Task {

template <LatticeType LR, PointSetType PST, Compress COMPRESS, PerLevelOrder PLO, class FIGURE>
struct RandomTag {};


/// Fully random search.
template <LatticeType LR, PointSetType PST, Compress COMPRESS, PerLevelOrder PLO, class FIGURE> using Random =
   LatSeqBasedSearch<RandomTag<LR, PST, COMPRESS, PLO, FIGURE>>;


/// Fully random search.
template <class FIGURE, LatticeType LR, PointSetType PST, Compress COMPRESS, PerLevelOrder PLO>
Random<LR, PST, COMPRESS, PLO, FIGURE> random(
      Storage<LR, PST, COMPRESS, PLO> storage,
      Dimension dimension,
      FIGURE figure,
      unsigned int numRand
      )
{ return Random<LR, PST, COMPRESS, PLO, FIGURE>(std::move(storage), dimension, std::move(figure), numRand); }


template <LatticeType LR, PointSetType PST, Compress COMPRESS, PerLevelOrder PLO, class FIGURE>
struct LatSeqBasedSearchTraits<RandomTag<LR, PST, COMPRESS, PLO, FIGURE>> {
   typedef LatBuilder::Task::Search<LR, PST> Search;
   typedef LatBuilder::Storage<LR, PST, COMPRESS, PLO> Storage;
   typedef typename LatBuilder::Storage<LR, PST, COMPRESS, PLO>::SizeParam SizeParam;
   typedef typename CBCSelector<LR, PST, COMPRESS, PLO, FIGURE>::CBC CBC;
   typedef LFSR113 RandomGenerator;
   typedef LatBuilder::Traversal::Random<RandomGenerator> Traversal;
   typedef GenSeq::GeneratingValues<LR, COMPRESS, Traversal> GenSeqType;
   typedef LatSeq::Combiner<LR, PST, GenSeqType, Zip> LatSeqType;

   LatSeqBasedSearchTraits(unsigned int numRand_):
      numRand(numRand_)
   {}

   virtual ~LatSeqBasedSearchTraits() {}

   LatSeqType latSeq(const SizeParam& sizeParam, Dimension dimension)
   {
      auto infty = std::numeric_limits<typename Traversal::size_type>::max();
      std::vector<GenSeqType> vec;
      vec.reserve(dimension);
      vec.push_back(GenSeq::Creator<GenSeqType>::create(SizeParam(LatticeTraits<LR>::TrivialModulus)));
      for (Dimension j = 1; j < dimension; j++) {
         vec.push_back(
               GenSeq::Creator<GenSeqType>::create(
                  sizeParam,
                  Traversal(infty, rand)
                  )
               );
         rand.jump();
      }
      return LatSeqType(sizeParam, std::move(vec));
   }

   std::string name() const
   { return FIGURE::evaluationName() + " random search (" + boost::lexical_cast<std::string>(numRand) + " random samples)"; }

   void init(LatBuilder::Task::Random<LR, PST, COMPRESS, PLO, FIGURE>& search) const
   {
      connectCBCProgress(search.cbc(), search.minObserver(), search.filters().empty());
      search.minObserver().setMaxAcceptedCount(numRand);
   }

   unsigned int numRand;
   RandomGenerator rand;
};

TASK_FOR_ALL(TASK_EXTERN_TEMPLATE, LatSeqBasedSearch, Random);

}}

#endif
