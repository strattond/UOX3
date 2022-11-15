function onUseChecked( pUser, potionKeg )
{
	var socket = pUser.socket;
	var potionCount = parseInt( potionKeg.GetTag( "potionCount" ));
	if( pUser.InRange( pUser, 2 ))
	{
		//Check to make sure you are not invisible.
		if( pUser.visible == 1 || pUser.visible == 2 )
		{
			pUser.visible = 0;
		}

		//Check to see if it's locked down
		if( potionKeg.movable == 3 )
		{
			socket.SysMessage( GetDictionaryEntry( 774, socket.language )); //That is locked down and you cannot use it
			return false;
		}

		// Checking potion Count
		if ( potionCount > 0 )
		{
			var resource = pUser.ResourceCount( 0x0f0e );// Checking Make sure we have a bottle.
			if( resource >= 0 )
			{
				var potionSectionID = potionKeg.GetTag( "potionSectionID" );
				var newPotion = CreateDFNItem( socket, pUser, potionSectionID, 1, "ITEM", true );
				if( ValidateObject( newPotion )) // Always validate that the item got created properly
				{
					pUser.SysMessage( GetDictionaryEntry( 17201, pSocket.language ) );//You pour some of the keg's contents into an empty bottle...

					// Reduce potionCount in keg
					var potionCount = parseInt( potionKeg.GetTag( "potionCount" ));
					potionKeg.SetTag( "potionCount", potionCount - 1 );
					pUser.UseResource( 1, 0x0f0e );
					pUser.SysMessage( GetDictionaryEntry( 17202, pSocket.language ) );// ...and place it into your backpack.
					pUser.SoundEffect( 0x240, true );
					potionKeg.weight -= Math.round(newPotion.weight / 5 );
					potionKeg.Refresh();
				}
			}
			else
				pUser.SysMessage( GetDictionaryEntry( 17203, pSocket.language ) );//you have no empty bottles.
		}
		else 
		{
			pUser.SysMessage( GetDictionaryEntry( 17211, pSocket.language ) );//The keg is empty.
			return;
		}
	}
	else
		pUser.SysMessage( GetDictionaryEntry( 17204, pSocket.language ) );//I can't reach that.
	return false;
}

const maxPotion = 100;
function onDropItemOnItem( iDropped, pUser, potionKeg )
{
	var socket = pUser.socket;
	var potionCount = parseInt( potionKeg.GetTag( "potionCount" ));
	var potionSectionID = potionKeg.GetTag( "potionSectionID" );
	if( ( !ValidateObject( potionKeg.owner ) || potionKeg.owner != pUser ) && !pUser.isGM )
	{
		pSocket.SysMessage( GetDictionaryEntry( 17205, pSocket.language ) );//Only the owner can drop potions in this keg.
		return;
	}

    // Make sure there's space in the poitonKeg for the new rune
	if( potionCount >= maxPotion )
	{
		pUser.SysMessage( GetDictionaryEntry( 17206, pSocket.language ) );//The keg will not hold any more!
		return false;
	}

	if( iDropped.type == 19 ) //If the item being dropped is a potion
	{
		if( potionSectionID == "" || iDropped.sectionID == potionSectionID ) // Check to see if it is the same potion.
		{
			// Check that the player's backpack can contain a empty bottle
			var pPack = pUser.pack;
			if( pPack.totalItemCount >= pPack.maxItems || pPack.weight >= pPack.weightMax ) 
			{
				pSocket.SysMessage( GetDictionaryEntry( 17207, pSocket.language ) );//You don't have room for the empty bottle in your backpack.
				return false;
			}
			else 
			{
				// Add Potion to Cage and Delete the Bottle.
				potionKeg.SetTag( "potionSectionID", iDropped.sectionID );
				potionKeg.SetTag( "potionCount", potionCount + 1 );
				potionKeg.weight += Math.round( iDropped.weight / 5 );
				potionKeg.name = "A keg of " + iDropped.name;
				if( iDropped.amount > 1 )
				{
					iDropped.amount = ( iDropped.amount - 1 );
				}
				else
				{
					iDropped.Delete();
				}

				//creates a empty bottle make the potion keg be owned by the dropper and now refresh keg.
				CreateDFNItem( socket, pUser, "0x0f0e", 1, "ITEM", true )
				pUser.TextMessage( GetDictionaryEntry( 17208, pSocket.language ) );//You place the empty bottle in your backpack.
				pUser.SoundEffect( 0x240, true );
				potionKeg.owner = pUser;
				potionKeg.Refresh();
				return 2;
			}
		}
		else 
		{
			pUser.TextMessage( GetDictionaryEntry( 17209, pSocket.language ) );//You decide that it would be a bad idea to mix different types of potions.
			return false;
		}
	}
	//else
		//pUser.TextMessage( GetDictionaryEntry( 17210, pSocket.language ) );//The keg is not designed to hold that type of object.
	return true;
}

function onTooltip( potionKeg )
{
	var tooltipText = "";
	var potionCount = parseInt( potionKeg.GetTag( "potionCount" ));
	if( potionCount <= 0 )
	{
		tooltipText = ( GetDictionaryEntry( 17211, pSocket.language ) );//The keg is empty.
		potionKeg.name = "a potion keg";
		potionKeg.SetTag("potionSectionID", null);
		return tooltipText;
	}
	else if( potionCount < 5 )
	{
		tooltipText = ( GetDictionaryEntry( 17212, pSocket.language ) ); return tooltipText;//The keg is nearly empty.
	}
	else if( potionCount < 20 )
	{
		tooltipText = ( GetDictionaryEntry( 17213, pSocket.language ) ); return tooltipText;//The keg is not very full.
	}
	else if( potionCount < 30 )
	{
		tooltipText = ( GetDictionaryEntry( 17214, pSocket.language ) ); return tooltipText;//The keg is about one quarter full.
	}
	else if( potionCount < 40 )
	{
		tooltipText = ( GetDictionaryEntry( 17215, pSocket.language ) ); return tooltipText;//The keg is about one third full.
	}
	else if( potionCount < 47 )
	{
		tooltipText = ( GetDictionaryEntry( 17216, pSocket.language ) ); return tooltipText;//The keg is almost half full.
	}
	else if( potionCount < 54 )
	{
		tooltipText = ( GetDictionaryEntry( 17217, pSocket.language ) ); return tooltipText;//The keg is approximately half full.
	}
	else if( potionCount < 70 )
	{
		tooltipText = ( GetDictionaryEntry( 17218, pSocket.language ) ); return tooltipText;//The keg is more than half full.
	}
	else if( potionCount < 80 ) 
	{
		tooltipText = ( GetDictionaryEntry( 17219, pSocket.language ) ); return tooltipText;//The keg is about three quarters full.
	}
	else if( potionCount < 96 )
	{
		tooltipText = ( GetDictionaryEntry( 17220, pSocket.language ) ); return tooltipText;//The keg is very full.
	}
	else if( potionCount < 100 )
	{
		tooltipText = ( GetDictionaryEntry( 17221, pSocket.language ) ); return tooltipText;//The liquid is almost to the top of the keg.
	}
	else 
	{
		tooltipText = ( GetDictionaryEntry( 17222, pSocket.language ) ); return tooltipText;//The keg is completely full.
	}
	return tooltipText;
}