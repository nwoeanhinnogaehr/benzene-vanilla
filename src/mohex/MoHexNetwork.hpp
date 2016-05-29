//----------------------------------------------------------------------------
/** @file MoHexNetwork.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXNETWORK_HPP
#define MOHEXNETWORK_HPP

#include "networkEvaluator.h"
#include "Benzene.hpp"
#include "BoardUtil.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class MoHexNetwork
{
    public:
        MoHexNetwork();

        void Evaluate(StoneBoard &board, HexColor toPlay, double *scores);

    protected:
        networkEvaluator m_eval;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXNETWORK_HPP
