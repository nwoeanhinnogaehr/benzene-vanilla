//----------------------------------------------------------------------------
/** @file HexSgUtil.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgNode.h"
#include "SgProp.h"
#include "SgGameWriter.h"

#include "Hex.hpp"
#include "HexProp.hpp"
#include "HexSgUtil.hpp"
#include "BitsetIterator.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

SgPoint HexSgUtil::HexPointToSgPoint(HexPoint p, int height)
{
    int c, r;
    HexPointUtil::pointToCoords(p, c, r);
    return SgPointUtil::Pt(1+c, height - r);
}


HexPoint HexSgUtil::SgPointToHexPoint(SgPoint p, int height)
{
    int c = SgPointUtil::Col(p);
    int r = SgPointUtil::Row(p);
    return HexPointUtil::coordsToPoint(c-1, height - r);
}


SgBlackWhite HexSgUtil::HexColorToSgColor(HexColor color)
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    if (color == BLACK) return SG_BLACK;
    return SG_WHITE;
}


HexColor HexSgUtil::SgColorToHexColor(SgBlackWhite player)
{
    HexAssert(player == SG_BLACK || player == SG_WHITE);
    if (player == SG_BLACK) return BLACK;
    return WHITE;
}

SgVector<SgPoint> HexSgUtil::BitsetToSgVector(const bitset_t& b, int height)
{
    SgVector<SgPoint> ret;
    std::vector<HexPoint> hp;
    for (BitsetIterator i(b); i; ++i)
        ret.Append(HexSgUtil::HexPointToSgPoint(*i, height));
    return ret;
}

void HexSgUtil::AddMoveToNode(SgNode* node, HexColor color, 
                              HexPoint cell, int height)
{
    SgPoint sgcell = HexSgUtil::HexPointToSgPoint(cell, height); 
    SgBlackWhite sgcolor = HexSgUtil::HexColorToSgColor(color);
    HexProp::AddMoveProp(node, sgcell, sgcolor);
}

bool HexSgUtil::NodeHasSetupInfo(SgNode* node)
{
    if (node->HasProp(SG_PROP_ADD_BLACK)) return true;
    if (node->HasProp(SG_PROP_ADD_WHITE)) return true;
    if (node->HasProp(SG_PROP_ADD_EMPTY)) return true;
    if (node->HasProp(SG_PROP_PLAYER)) return true;
    return false;
}

void HexSgUtil::SetPositionInNode(SgNode* node, const StoneBoard& brd, 
                                  HexColor color)
{
    int height = brd.height();
    SgVector<SgPoint> blist = HexSgUtil::BitsetToSgVector(brd.getBlack() 
                                       & brd.Const().getCells(), height);
    SgVector<SgPoint> wlist = HexSgUtil::BitsetToSgVector(brd.getWhite()
                                       & brd.Const().getCells(), height);
    SgVector<SgPoint> elist = HexSgUtil::BitsetToSgVector(brd.getEmpty()
                                       & brd.Const().getCells(), height);

    SgPropPlayer* pprop = new SgPropPlayer(SG_PROP_PLAYER);
    SgPropAddStone* bprop = new SgPropAddStone(SG_PROP_ADD_BLACK);
    SgPropAddStone* wprop = new SgPropAddStone(SG_PROP_ADD_WHITE);
    SgPropAddStone* eprop = new SgPropAddStone(SG_PROP_ADD_EMPTY);

    pprop->SetValue(HexSgUtil::HexColorToSgColor(color));    
    bprop->SetValue(blist);
    wprop->SetValue(wlist);
    eprop->SetValue(elist);

    node->Add(pprop);
    node->Add(bprop);
    node->Add(wprop);
    node->Add(eprop);
}

void HexSgUtil::GetSetupPosition(const SgNode* node, int height, 
                                 std::vector<HexPoint>& black,
                                 std::vector<HexPoint>& white,
                                 std::vector<HexPoint>& empty)
{
    black.clear();
    white.clear();
    empty.clear();
    if (node->HasProp(SG_PROP_ADD_BLACK)) 
    {
        SgPropPointList* prop = (SgPropPointList*)node->Get(SG_PROP_ADD_BLACK);
        const SgVector<SgPoint>& vec = prop->Value();
        for (int i = 0; i < vec.Length(); ++i)
            black.push_back(HexSgUtil::SgPointToHexPoint(vec[i], height));
    }
    if (node->HasProp(SG_PROP_ADD_WHITE)) 
    {
        SgPropPointList* prop = (SgPropPointList*)node->Get(SG_PROP_ADD_WHITE);
        const SgVector<SgPoint>& vec = prop->Value();
        for (int i = 0; i < vec.Length(); ++i)
            white.push_back(HexSgUtil::SgPointToHexPoint(vec[i], height));
    }
    if (node->HasProp(SG_PROP_ADD_EMPTY)) 
    {
        SgPropPointList* prop = (SgPropPointList*)node->Get(SG_PROP_ADD_EMPTY);
        const SgVector<SgPoint>& vec = prop->Value();
        for (int i = 0; i < vec.Length(); ++i)
            empty.push_back(HexSgUtil::SgPointToHexPoint(vec[i], height));
    }
}

void HexSgUtil::SetPositionInBoard(const SgNode* node, StoneBoard& brd)
{
    std::vector<HexPoint> black, white, empty;
    GetSetupPosition(node, brd.height(), black, white, empty);
    brd.startNewGame();
    for (std::size_t i = 0; i < black.size(); ++i) 
        brd.playMove(BLACK, black[i]);
    for (std::size_t i = 0; i < white.size(); ++i)
        brd.playMove(WHITE, white[i]);
}


bool HexSgUtil::WriteSgf(SgNode* tree, const std::string& application,
                         const char* filename, int boardsize)
{
    // Set the boardsize property
    tree->Add(new SgPropInt(SG_PROP_SIZE, boardsize));

    // @note 11 is the sgf gamenumber for Hex
    std::ofstream f(filename);
    if (f) 
    {
        SgGameWriter sgwriter(f);
        sgwriter.WriteGame(*tree, true, 0, application, 11, boardsize);
        f.close();
    } 
    else 
    {
        LogWarning() << "Could not open '" << filename << "' "
                     << "for writing!\n";
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------
