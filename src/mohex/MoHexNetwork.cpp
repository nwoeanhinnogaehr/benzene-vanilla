//----------------------------------------------------------------------------
/** @file MoHexNetwork.cpp */
//----------------------------------------------------------------------------

#include "MoHexNetwork.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

MoHexNetwork::MoHexNetwork() : m_eval() {}

void MoHexNetwork::Evaluate(StoneBoard &board, HexColor toPlay, double *scores, std::vector<int> diff)
{
    // 13x13 only for now
    const int boardN = 13;
    const int boardSize = boardN * boardN;
    BenzeneAssert(board.Width() == boardN && board.Height() == boardN);

    bool netState[2 * boardSize];
    double netScores[boardSize];
    for (int i = 0; i < boardSize * 2; i++) {
        int x = (i % boardN);
        int y = (i / boardN) % boardN;
        HexPoint point = HexPointUtil::coordsToPoint(y, x);
        HexColor color = static_cast<HexColor>(1 - i / boardSize);
        netState[i] = board.IsColor(point, color);
    }

    for (size_t i = 0; i < diff.size(); i++) {
        netState[diff[i]] = true;
    }

    m_eval.evaluate(netState, 1 - toPlay, netScores);

    for (int x = 0; x < boardN; x++)
        for (int y = 0; y < boardN; y++)
            scores[x + y * boardN] = netScores[y + x * boardN];
}

//----------------------------------------------------------------------------
