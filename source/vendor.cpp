#include "uox3.h"
#include <algorithm>
#include "targeting.h"
#include "commands.h"
#include "cEffects.h"
#include "CPacketSend.h"
#include "classes.h"
#include "townregion.h"

#undef DBGFILE
#define DBGFILE "vendor.cpp"

#include "ObjectFactory.h"

namespace UOX
{

//o---------------------------------------------------------------------------o
//|	Function	-	UI32 calcValue( CItem *i, UI32 value )
//|	Programmer	-	UOX3 DevTeam
//o---------------------------------------------------------------------------o
//|	Purpose		-	Calculate the value of an item
//o---------------------------------------------------------------------------o
UI32 calcValue( CItem *i, UI32 value )
{
	UI32 mod = 10;

	if( i->GetType() == IT_POTION )
	{
		if( i->GetTempVar( CITV_MOREX ) > 500 )
			++mod;
		if( i->GetTempVar( CITV_MOREX ) > 900 )
			++mod;
		if( i->GetTempVar( CITV_MOREX ) > 1000 )
			++mod;
		if( i->GetTempVar( CITV_MOREZ ) > 1 )
			mod += (3*(i->GetTempVar( CITV_MOREZ )-1));
		value=(value*mod)/10;
	}

	if( i->GetRank() > 0 && i->GetRank() < 10 && cwmWorldState->ServerData()->RankSystemStatus() )
		value = (UI32)( i->GetRank() * value ) / 10;
	if( value < 1 )
		value = 1;

	if( !i->GetRndValueRate() )
		i->SetRndValueRate( 0 );
	if( i->GetRndValueRate() != 0 && cwmWorldState->ServerData()->TradeSystemStatus() )
		value += (UI32)(value*i->GetRndValueRate())/1000;
	if( value < 1 )
		value = 1;
	return value;
}

//o---------------------------------------------------------------------------o
//|	Function	-	UI32 calcGoodValue( CChar *npcnum2, CItem *i, UI32 value, bool isSelling )
//|	Programmer	-	UOX3 DevTeam
//o---------------------------------------------------------------------------o
//|	Purpose		-	Calculate the value of a good
//o---------------------------------------------------------------------------o
UI32 calcGoodValue( CChar *npcnum2, CItem *i, UI32 value, bool isSelling )
{
	cTownRegion *tReg	= calcRegionFromXY( npcnum2->GetX(), npcnum2->GetY(), npcnum2->WorldNumber() );
	SI16 good			= i->GetGood();
	SI32 regvalue		= 0;

	if( good <= -1 )
		return value;

	if( isSelling )
		regvalue = tReg->GetGoodSell( static_cast<UI08>(i->GetGood()) );
	else
		regvalue = tReg->GetGoodBuy( static_cast<UI08>(i->GetGood()) );

	UI32 x = (UI32)( value * abs( regvalue ) ) / 1000;

	if( regvalue < 0 )
		value -= x;
	else
		value += x;

	if( value <= 0 )
		value = 1;

	return value;
}

//o---------------------------------------------------------------------------o
//|   Function    :  void buyItem( cSocket *mSock )
//|   Date        :  Unknown
//|   Programmer  :  UOX3 DevTeam
//o---------------------------------------------------------------------------o
//|   Purpose     :  Called when player buys an item from a vendor
//o---------------------------------------------------------------------------o
bool CPIBuyItem::Handle( void )
{
	UI16 i, j;
	UI32 playergoldtotal, goldtotal = 0;
	bool soldout	= false, clear = false;
	CChar *mChar	= tSock->CurrcharObj();
	CItem *p		= mChar->GetPackItem();
	if( !ValidateObject( p ) ) 
		return true;

	ITEMLIST bitems;
	std::vector< UI08 > layer;
	std::vector< UI16 > amount;

	CChar *npc		= calcCharObjFromSer( tSock->GetDWord( 3 ) );
	UI16 itemtotal	= static_cast<UI16>((tSock->GetWord( 1 ) - 8) / 7);
	if( itemtotal > 511 ) 
		return true;
	bitems.resize( itemtotal );
	amount.resize( itemtotal );
	layer.resize( itemtotal );
	int baseOffset = 0;
	for( i = 0; i < itemtotal; ++i )
	{
		baseOffset	= 7 * i;
		layer[i]	= tSock->GetByte( 8 + baseOffset );
		bitems[i]	= calcItemObjFromSer( tSock->GetDWord( 9 + baseOffset ) );
		amount[i]	= tSock->GetWord( 13 + baseOffset );
		goldtotal	+= ( amount[i] * ( bitems[i]->GetBuyValue() ) );
	}
	bool useBank = (goldtotal >= static_cast<UI32>(cwmWorldState->ServerData()->BuyThreshold() ));
	if( useBank )
		playergoldtotal = GetBankCount( mChar, 0x0EED );
	else
		playergoldtotal = GetItemAmount( mChar, 0x0EED );
	if( playergoldtotal >= goldtotal || mChar->IsGM() )
	{
		for( i = 0; i < itemtotal; ++i )
		{
			if( bitems[i]->GetAmount() < amount[i] )
				soldout = true;
		}
		if( soldout )
		{
			npc->talk( tSock, 1336, false );
			clear = true;
		}
		else
		{
			if( mChar->IsGM() )
				npc->talkAll( 1337, false, mChar->GetName().c_str() );
			else
			{
				if( goldtotal == 1 )
					npc->talkAll( 1338, false, mChar->GetName().c_str(), goldtotal );
				else
					npc->talkAll( 1339, false, mChar->GetName().c_str(), goldtotal );

				Effects->goldSound( tSock, goldtotal );
			}
			
			clear = true;
			if( !mChar->IsGM() ) 
			{
				if( useBank )
					DeleteBankItem( mChar, goldtotal, 0x0EED );
				else
					DeleteItemAmount( mChar, goldtotal, 0x0EED );
			}
			CItem *biTemp;
			CItem *iMade = NULL;
			for( i = 0; i < itemtotal; ++i )
			{
				biTemp	= bitems[i];
				iMade	= NULL;
				if( biTemp->GetAmount() > amount[i] )
				{
					if( biTemp->isPileable() )
					{
						iMade = Commands->DupeItem( tSock, biTemp, amount[i] );
						if( iMade != NULL )
						{
							iMade->SetCont( p );
							iMade->PlaceInPack();
						}
					}
					else
					{
						for( j = 0; j < amount[i]; ++j )
						{
							iMade = Commands->DupeItem( tSock, biTemp, 1 );
							if( iMade != NULL )
							{
								iMade->SetCont( p );
								iMade->PlaceInPack();
							}
						}
					}
					biTemp->IncAmount( -amount[i] );
					biTemp->SetRestock( biTemp->GetRestock() + amount[i] );
				}
				else
				{
					switch( layer[i] )
					{
						case 0x1A: // Buy Container
							if( biTemp->isPileable() )
							{
								iMade = Commands->DupeItem( tSock, biTemp, amount[i] );
								if( iMade != NULL )
								{
									iMade->SetCont( p );
									iMade->PlaceInPack();
								}
							}
							else
							{
								for( j = 0; j < amount[i]; ++j )
								{
									iMade = Commands->DupeItem( tSock, biTemp, 1 );
									if( iMade != NULL )
									{
										iMade->SetCont( p );
										iMade->PlaceInPack();
									}
								}
								iMade = NULL;
							}
							biTemp->IncAmount( -amount[i] );
							biTemp->SetRestock( biTemp->GetRestock() + amount[i] );
							break;
						case 0x1B: // Bought Container
							if( biTemp->isPileable() )
								biTemp->SetCont( p );
							else
							{
								for( j = 0; j < amount[i]-1; ++j )
								{
									iMade = Commands->DupeItem( tSock, biTemp, 1 );
									if( iMade != NULL )
									{
										iMade->SetCont( p );
										iMade->PlaceInPack();
									}
								}
								biTemp->SetCont( p );
								biTemp->SetAmount( 1 );
							}
							break;
						default:
							Console.Error( 2, " Fallout of switch statement without default. vendor.cpp, buyItem()" );
							break;
					}
				}
			}
		}
	}
	else
		npc->talkAll( 1340, false );
	
	if( clear )
	{
		CPBuyItem clrSend;
		clrSend.Serial( tSock->GetDWord( 3 ) );
		tSock->Send( &clrSend );
	}
	tSock->statwindow( mChar );
	return true;
}

//o---------------------------------------------------------------------------o
//|   Function    :  void sendSellSubItem( CChar *npc, CItem *p, CItem *q, UI08 *m1, int &m1t)
//|   Date        :  Unknown
//|   Programmer  :  Unknown
//o---------------------------------------------------------------------------o
//|   Purpose     :  Send available sell items to client for Sell list
//o---------------------------------------------------------------------------o
void sendSellSubItem( CChar *npc, CItem *p, CItem *q, UI08 *m1, int &m1t)
{
	UI32 value;
	int z;
	char ciname[MAX_NAME];
	char cinam2[MAX_NAME];
	std::string itemname;
	itemname.reserve( MAX_NAME );
	
	for( CItem *i = p->FirstItem(); !p->FinishedItems(); i = p->NextItem() )
	{
		if( ValidateObject( i ) )
		{
			strcpy( ciname, i->GetName().c_str() );
			strcpy( cinam2, q->GetName().c_str() );
			strupr( ciname );
			strupr( cinam2 );
			
			if( i->GetType() == IT_CONTAINER )
				sendSellSubItem( npc, i, q, m1, m1t );
			else if( i->GetID() == q->GetID() && i->GetType() == q->GetType() &&
				m1[8] < 60 && ( !cwmWorldState->ServerData()->SellByNameStatus() ||
				( cwmWorldState->ServerData()->SellByNameStatus() && !strcmp( ciname, cinam2 ) ) ) )
			{
				m1[m1t+0] = i->GetSerial( 1 );
				m1[m1t+1] = i->GetSerial( 2 );
				m1[m1t+2] = i->GetSerial( 3 );
				m1[m1t+3] = i->GetSerial( 4 );
				m1[m1t+4] = i->GetID( 1 );
				m1[m1t+5] = i->GetID( 2 );
				m1[m1t+6] = i->GetColour( 1 );
				m1[m1t+7] = i->GetColour( 2 );
				m1[m1t+8] = (UI08)(i->GetAmount()>>8);
				m1[m1t+9] = (UI08)(i->GetAmount()%256);
				value = calcValue( i, q->GetBuyValue() );
				if( cwmWorldState->ServerData()->TradeSystemStatus() )
					value = calcGoodValue( npc, q, value, true );
				m1[m1t+10] = (UI08)(value>>8);
				m1[m1t+11] = (UI08)(value%256);
				m1[m1t+12] = 0;// Unknown... 2nd length byte for string?
				m1[m1t+13] = (UI08)(getTileName( i, itemname ));
				m1t += 14;
				for( z = 0; z < m1[m1t-1]; ++z )
					m1[m1t+z] = itemname[z];

				m1t += m1[m1t-1];
				++m1[8];
			}
		}
	}
}

//o---------------------------------------------------------------------------o
//|   Function    :  bool sendSellStuff( cSocket *s, CChar *i )
//|   Date        :  Unknown
//|   Programmer  :  UOX3 DevTeam
//o---------------------------------------------------------------------------o
//|   Purpose     :  Send Vendor Sell List to client
//o---------------------------------------------------------------------------o
bool sendSellStuff( cSocket *s, CChar *i )
{
	std::string itemname;
	UI32 value;
	UI08 m1[2048];
	char ciname[256];
	char cinam2[256];
	
	CItem *vendorPack = i->GetItemAtLayer( IL_SELLCONTAINER );	// find the acceptable sell layer
	if( !ValidateObject( vendorPack ) ) 
		return false;	// no layer
	
	CPPauseResume prPause( 1 );
	CChar *mChar = s->CurrcharObj();
	s->Send( &prPause );
	
	CItem *pack = mChar->GetPackItem();				// no pack for the player
	if( !ValidateObject( pack ) ) 
		return false;
	
	m1[0] = 0x9E; // Header
	m1[1] = 0; // Size
	m1[2] = 0; // Size
	m1[3] = i->GetSerial( 1 );
	m1[4] = i->GetSerial( 2 );
	m1[5] = i->GetSerial( 3 );
	m1[6] = i->GetSerial( 4 );
	m1[7] = 0; // Num items
	m1[8] = 0; // Num items
	int m1t = 9;

	for( CItem *q = vendorPack->FirstItem(); !vendorPack->FinishedItems(); q = vendorPack->NextItem() )
	{
		if( ValidateObject( q ) )
		{
			for( CItem *j = pack->FirstItem(); !pack->FinishedItems(); j = pack->NextItem() )
			{
				if( ValidateObject( j ) )
				{
					strcpy( ciname, j->GetName().c_str() );
					strcpy( cinam2, q->GetName().c_str() );
					strupr( ciname );
					strupr( cinam2 );
					if( j->GetType() == IT_CONTAINER )
						sendSellSubItem( i, j, q, m1, m1t );
					else if( ( j->GetID() == q->GetID() && j->GetType() == q->GetType() &&
						m1[8] < 60 && !cwmWorldState->ServerData()->SellByNameStatus() ) || ( cwmWorldState->ServerData()->SellByNameStatus() && 
						!strcmp( ciname, cinam2 ) ) )
					{
						m1[m1t+0] = j->GetSerial( 1 );
						m1[m1t+1] = j->GetSerial( 2 );
						m1[m1t+2] = j->GetSerial( 3 );
						m1[m1t+3] = j->GetSerial( 4 );
						m1[m1t+4] = j->GetID( 1 );
						m1[m1t+5] = j->GetID( 2 );
						m1[m1t+6] = j->GetColour( 1 );
						m1[m1t+7] = j->GetColour( 2 );
						m1[m1t+8] = (UI08)(j->GetAmount()>>8);
						m1[m1t+9] = (UI08)(j->GetAmount()%256);
						value = calcValue( j, q->GetBuyValue() );
						if( cwmWorldState->ServerData()->TradeSystemStatus() )
							value = calcGoodValue( i, j, value, true );
						m1[m1t+10] = (UI08)(value>>8);
						m1[m1t+11] = (UI08)(value%256);
						m1[m1t+12] = 0;// Unknown... 2nd length byte for string?
						m1[m1t+13] = (UI08)(getTileName( j, itemname ));
						m1t += 14;
						for( int z = 0; z < m1[m1t-1]; ++z )
							m1[m1t+z]=itemname[z];

						m1t += m1[m1t-1];
						++m1[8];
					}
				}
			}
		}
	}
	
	m1[1] = (UI08)(m1t>>8);
	m1[2] = (UI08)(m1t%256);
	if( m1[8] != 0 )
	{
		s->Send( m1, m1t );
#if defined( _MSC_VER )
#pragma note( "Flush location" )
#endif
		s->FlushBuffer();
	}
	else
		i->talkAll( 1341, false );
	CPPauseResume prResume( 0 );
	s->Send( &prResume );
	return true;
}

//o---------------------------------------------------------------------------o
//|   Function    :  void sellItem( cSocket *mSock )
//|   Date        :  Unknown
//|   Programmer  :  UOX3 DevTeam
//o---------------------------------------------------------------------------o
//|   Purpose     :  Player sells an item to the vendor
//o---------------------------------------------------------------------------o
bool CPISellItem::Handle( void )
{
	if( tSock->GetByte( 8 ) != 0 )
	{
		CChar *mChar	= tSock->CurrcharObj();
		CChar *n		= calcCharObjFromSer( tSock->GetDWord( 3 ) );
		if( !ValidateObject( n ) || !ValidateObject( mChar ) )
			return true;
		CItem *buyPack		= n->GetItemAtLayer( IL_BUYCONTAINER );
		CItem *boughtPack	= n->GetItemAtLayer( IL_BOUGHTCONTAINER );
		CItem *sellPack		= n->GetItemAtLayer( IL_SELLCONTAINER );
		CItem *j, *k;
		UI16 i, amt, maxsell = 0;
		UI32 totgold = 0, value = 0;
		for( i = 0; i < tSock->GetByte( 8 ); ++i )
		{
			j = calcItemObjFromSer( tSock->GetDWord( 9 + (6*i) ) );
			amt = tSock->GetWord( 13 + (6*i) );
			maxsell += amt;
		}

		if( maxsell > cwmWorldState->ServerData()->SellMaxItemsStatus() )
		{
			n->talkAll( 1342, false, mChar->GetName().c_str(), cwmWorldState->ServerData()->SellMaxItemsStatus() );
			return true;
		}

		for( i = 0; i < tSock->GetByte( 8 ); ++i )
		{
			j = calcItemObjFromSer( tSock->GetDWord( 9 + (6*i) ) );
			amt = tSock->GetWord( 13 + (6*i) );
			if( ValidateObject( j ) )
			{
				if( j->GetAmount() < amt )
				{
					n->talkAll( 1343, false );
					return true;
				}
				CItem *join = NULL;
				for( k = sellPack->FirstItem(); !sellPack->FinishedItems(); k = sellPack->NextItem() )
				{
					if( ValidateObject( k ) )
					{
						if( k->GetID() == j->GetID() && j->GetType() == k->GetType() )
							join = k;
					}
				}
				for( k = buyPack->FirstItem(); !buyPack->FinishedItems(); k = buyPack->NextItem() )
				{
					if( ValidateObject( k ) )
					{
						if( k->GetID() == j->GetID() && j->GetType() == k->GetType() )
							value = calcValue( j, k->GetBuyValue() );
					}
				}
				if( ValidateObject( join ) )
				{
					join->IncAmount( amt );
					join->SetRestock( join->GetRestock() - amt );
					totgold += ( amt * value );
					if( j->GetAmount() == amt )
						j->Delete();
					else
						j->IncAmount( -amt );
				}
				else
				{
					totgold += ( amt * value );
					j->SetCont( boughtPack );

					if( j->GetAmount() != amt ) 
						Commands->DupeItem( tSock, j, j->GetAmount() - amt );
				}
			} 
		}
		Items->CreateItem( tSock, mChar, 0x0EED, totgold, 0, OT_ITEM, true );
		Effects->goldSound( tSock, totgold );
	}
	
	CPBuyItem clrSend;
	clrSend.Serial( tSock->GetDWord( 3 ) );
	tSock->Send( &clrSend );
	return true;
}

//o---------------------------------------------------------------------------o
//|   Function    :  void restockNPC( CChar *i, bool stockAll )
//|   Date        :  Unknown
//|   Programmer  :  UOX3 DevTeam
//o---------------------------------------------------------------------------o
//|   Purpose     :  Restock NPC Vendors
//o---------------------------------------------------------------------------o
void restockNPC( CChar *i, bool stockAll )
{
	if( !ValidateObject( i ) || !i->IsShop() )
		return;	// if we aren't a shopkeeper, why bother?

	CItem *ci = i->GetItemAtLayer( IL_BUYCONTAINER );
	if( ValidateObject( ci ) )
	{
		for( CItem *c = ci->FirstItem(); !ci->FinishedItems(); c = ci->NextItem() )
		{
			if( ValidateObject( c ) )
			{
				if( stockAll )
				{
					c->IncAmount( c->GetRestock() );
					c->SetRestock( 0 );
				}
				else if( c->GetRestock() )
				{
					UI16 stockAmt = UOX_MIN( c->GetRestock(), static_cast<UI16>(( c->GetRestock() / 2 ) + 1) );
					c->IncAmount( stockAmt );
					c->SetRestock( c->GetRestock() - stockAmt );
				}
				if( cwmWorldState->ServerData()->TradeSystemStatus() ) 
				{
					cTownRegion *tReg = calcRegionFromXY( i->GetX(), i->GetY(), i->WorldNumber() );
					Items->StoreItemRandomValue( c, tReg );
				}
			}
		}
	}
}

bool restockFunctor( cBaseObject *a, UI32 &b, void *extraData )
{
	bool retVal = true;
	if( ValidateObject( a ) )
	{
		CChar *c = static_cast< CChar * >(a);
		if( c->IsShop() )
			restockNPC( c, (b == 1) );
	}
	return retVal;
}
//o---------------------------------------------------------------------------o
//|   Function    :  void restock( bool stockAll )
//|   Date        :  3/15/2003
//|   Programmer  :	 Zane
//o---------------------------------------------------------------------------o
//|   Purpose     :  Restock all NPC Vendors
//o---------------------------------------------------------------------------o
void restock( bool stockAll )
{
	UI32 b = (stockAll ? 1 : 0);
	ObjectFactory::getSingleton().IterateOver( OT_CHAR, b, NULL, &restockFunctor );
}

}
