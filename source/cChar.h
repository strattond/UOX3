#ifndef __CCHAR_H__
#define __CCHAR_H__

namespace UOX
{

enum cC_TID
{
	// Global Character Timers
	tCHAR_TIMEOUT = 0,
	tCHAR_INVIS,
	tCHAR_HUNGER,
	tCHAR_POISONTIME,
	tCHAR_POISONTEXT,
	tCHAR_POISONWEAROFF,
	tCHAR_SPELLTIME,
	tCHAR_ANTISPAM,
	tCHAR_CRIMFLAG,
	tCHAR_MURDERRATE,
	// NPC Timers
	tNPC_MOVETIME,
	tNPC_SPATIMER,
	tNPC_SUMMONTIME,
	// PC Timers
	tPC_LOGOUT,
	tCHAR_COUNT
};

class CChar : public cBaseObject
{
// Base Characters
protected:
	SI08		hunger;		// Level of hungerness, 6 = full, 0 = "empty"
	UI08		fixedlight; // Fixed lighting level (For chars in dungeons, where they dont see the night)
	UI08		town;       // Matches Region number in regions.scp
	UI08		regionNum;

	UI32		bools;	// lots of flags
	UI16		priv;

	SI08		townpriv;  //0=non resident (Other privledges added as more functionality added)
	SERIAL		townvote;

	SI16		guildnumber;		// Number of guild player is in (0=no guild)			(DasRaetsel)
	SERIAL		guildfealty;	// Serial of player you are loyal to (default=yourself)	(DasRaetsel)
	std::string	guildtitle;	// Title Guildmaster granted player						(DasRaetsel)

	TIMERVAL	charTimers[tCHAR_COUNT];

	TIMERVAL	regen[3];
	TIMERVAL	weathDamage[WEATHNUM];			// Light Damage timer
	UI08		nextact;						//time to next spell action..

	SI08		fonttype; // Speech font to use
	COLOUR		saycolor;
	COLOUR		emotecolor;

	SI08		dispz; 
	SI08		stealth; // stealth ( steps already done, -1=not using )
	SI08		cell; // Reserved for jailing players
	UI08		running; // Stamina Loose while running
	UI08		step;						// 1 if step 1 0 if step 2 3 if step 1 skip 2 if step 2 skip
		
	CItem *		packitem; // Characters backpack
	SERIAL		targ; // Current combat target
	SERIAL		attacker; // Character who attacked this character
	UI16		advobj; //Has used advance gate?
	RACEID		raceGate;						// Race gate that has been used

	SI08		spellCast;

	SKILLVAL	baseskill[ALLSKILLS+1]; // Base skills without stat modifiers
	SKILLVAL	skill[ALLSKILLS+1]; // List of skills (with stat modifiers)
	UI16		atrophy[ALLSKILLS+1];
	UI08		lockState[ALLSKILLS+1];	// state of the skill locks

	UI16		deaths;
	UI08		flag; //1=red 2=grey 4=Blue 8=green 10=Orange	// should it not be 0x10??? sounds like we're trying to do
		
	bool		Saved;
	long		SavedAt;

	CItem *		itemLayers[MAXLAYERS];
	UI08		layerCtr;
	CDataList< CChar * >	petsControlled;
	ITEMLIST	ownedItems;
	UI32		skillUsed[2];	// no more than 64 skills

	UI16		maxHP;
	UI16		maxHP_oldstr;
	RACEID		maxHP_oldrace;
	SI16		maxMana;
	UI16		maxMana_oldint;
	RACEID		maxMana_oldrace;
	SI16		maxStam;
	UI16		maxStam_olddex;
	RACEID		maxStam_oldrace;

	UI08		PoisonStrength;

	virtual bool	DumpHeader( std::ofstream &outStream ) const;
	virtual bool	DumpBody( std::ofstream &outStream ) const;
	virtual bool	HandleLine( UString &UTag, UString &data );
	virtual bool	LoadRemnants( void );

	void		CopyData( CChar *target );

	SI16		oldLocX;
	SI16		oldLocY;
	SI08		oldLocZ;

public:

	point3		GetOldLocation( void );
	void		SetPoisonStrength( UI08 value );
	UI08		GetPoisonStrength( void ) const;

	CDataList< CChar * > *	GetPetList( void );
	ITEMLIST *	GetOwnedItems( void );

	void		AddOwnedItem( CItem *toAdd );
	void		RemoveOwnedItem( CItem *toRemove );

	SI08		GetHunger( void ) const;
	UI08		GetFixedLight( void ) const;
	UI08		GetTown( void ) const;

	void		SetFixedLight( UI08 newVal );
	void		SetHunger( SI08 newValue );
	void		SetTown( UI08 newValue );

	void		DecHunger( const SI08 amt = 1 );

	bool		isUnicode( void ) const;
	bool		IsNpc( void ) const;
	bool		IsShop( void ) const;
	bool		IsDead( void ) const;
	bool		IsAtWar( void ) const;
	bool		DidAttackFirst( void ) const;
	bool		IsOnHorse( void ) const;
	bool		GetTownTitle( void ) const;
	bool		GetReactiveArmour( void ) const;
	bool		CanTrain( void ) const;
	bool		GetGuildToggle( void ) const;
	bool		IsTamed( void ) const;
	bool		IsGuarded( void ) const;
	bool		CanRun( void ) const;
	bool		IsUsingPotion( void ) const;
	bool		MayLevitate( void ) const;
	bool		WillHunger( void ) const;
	bool		IsMeditating( void ) const;
	bool		IsCasting( void ) const;
	bool		IsJSCasting( void ) const;

	void		setUnicode( bool newVal );
	void		SetNpc( bool newVal );
	void		SetShop( bool newVal );
	void		SetDead( bool newValue );
	void		SetWar( bool newValue );
	void		SetAttackFirst( bool newValue );
	void		SetOnHorse( bool newValue );
	void		SetTownTitle( bool newValue );
	void		SetReactiveArmour( bool newValue );
	void		SetCanTrain( bool newValue );
	void		SetGuildToggle( bool newValue );
	void		SetTamed( bool newValue );
	void		SetGuarded( bool newValue );
	void		SetRun( bool newValue );
	void		SetUsingPotion( bool newVal );
	void		SetLevitate( bool newValue );
	void		SetHungerStatus( bool newValue );
	void		SetMeditating( bool newValue );
	void		SetCasting( bool newValue );
	void		SetJSCasting( bool newValue );

	void		SetTownVote( UI32 newValue );
	void		SetGuildFealty( UI32 newValue );

	UI32		GetTownVote( void ) const;
	UI32		GetGuildFealty( void ) const;

	std::string	GetGuildTitle( void ) const;
	void		SetGuildTitle( std::string newValue );

	TIMERVAL	GetTimer( cC_TID timerID ) const;
	TIMERVAL	GetRegen( UI08 part ) const;
	TIMERVAL	GetWeathDamage( UI08 part ) const;
	UI08		GetNextAct( void ) const;

	void		SetTimer( cC_TID timerID, TIMERVAL value );
	void		SetRegen( TIMERVAL newValue, UI08 part );
	void		SetWeathDamage( TIMERVAL newValue, UI08 part );
	void		SetNextAct( UI08 newVal );

	COLOUR		GetEmoteColour( void ) const;
	COLOUR		GetSayColour( void ) const;
	UI16		GetSkin( void ) const;

	void		SetSkin( UI16 value );
	void		SetEmoteColour( COLOUR newValue );
	void		SetSayColour( COLOUR newValue );

	SI08		GetDispZ( void ) const;
	SI08		GetStealth( void ) const;
	SI08		GetCell( void ) const;
	UI08		GetRunning( void ) const;
	UI08		GetStep( void ) const;
	cTownRegion *GetRegion( void ) const;
	UI08		GetRegionNum( void ) const;

	void			SetDispZ( SI08 newZ );
	void			SetCell( SI08 newVal );
	void			SetStealth( SI08 newValue );
	void			SetRunning( UI08 newValue );
	void			SetStep( UI08 newValue );
	void			SetRegion( UI08 newValue );
	virtual void	SetLocation( SI16 newX, SI16 newY, SI08 newZ, UI08 world );
	virtual void	SetLocation( SI16 newX, SI16 newY, SI08 newZ );
	virtual void	SetLocation( const cBaseObject *toSet );
	void			WalkXY( SI16 newX, SI16 newY );
	void			WalkZ( SI08 newZ );
	void			WalkDir( SI08 newDir );

	CItem *		GetPackItem( void );
	CChar *		GetTarg( void ) const;
	CChar *		GetAttacker( void ) const;
	UI16		GetAdvObj( void ) const;
	RACEID		GetRaceGate( void ) const;

	void		SetPackItem( CItem *newVal );
	void		SetTarg( CChar *newTarg );
	void		SetAttacker( CChar *newValue );
	void		SetAdvObj( UI16 newValue );
	void		SetRaceGate( RACEID newValue );

	SI08		GetSpellCast( void ) const;
	void		SetSpellCast( SI08 newValue );

	UI16		GetPriv( void ) const;
	SI08		GetTownPriv( void ) const;
	bool		IsGM( void ) const;
	bool		CanBroadcast( void ) const;
	bool		IsInvulnerable( void ) const;
	bool		GetSingClickSer( void ) const;
	bool		NoSkillTitles( void ) const;
	bool		IsGMPageable( void ) const;
	bool		CanSnoop( void ) const;
	bool		IsCounselor( void ) const;
	bool		AllMove( void ) const;
	bool		IsFrozen( void ) const;
	bool		ViewHouseAsIcon( void ) const;
	bool		NoNeedMana( void ) const;
	bool		IsDispellable( void ) const;
	bool		IsPermReflected( void ) const;
	bool		NoNeedReags( void ) const;

	void		SetGM( bool newValue );
	void		SetBroadcast( bool newValue );
	void		SetInvulnerable( bool newValue );
	void		SetSingClickSer( bool newValue );
	void		SetSkillTitles( bool newValue );
	void		SetGMPageable( bool newValue );
	void		SetSnoop( bool newValue );
	void		SetCounselor( bool newValue );
	void		SetAllMove( bool newValue );
	void		SetFrozen( bool newValue );
	void		SetViewHouseAsIcon( bool newValue );
	void		SetNoNeedMana( bool newValue );
	void		SetDispellable( bool newValue );
	void		SetPermReflected( bool newValue );
	void		SetNoNeedReags( bool newValue );

	void		SetPriv( UI16 newValue );
	void		SetTownpriv( SI08 newValue );

	UI16		GetBaseSkill( UI08 skillToGet ) const;
	UI16		GetAtrophy( UI08 skillToGet ) const;
	UI08		GetSkillLock( UI08 skillToGet ) const;
	UI16		GetSkill( UI08 skillToGet ) const;

	void		SetBaseSkill( UI16 newSkillValue, UI08 skillToSet );
	void		SetSkill( UI16 newSkillValue, UI08 skillToSet );
	void		SetAtrophy( UI16 newValue, UI08 skillToSet );
	void		SetSkillLock( UI08 newSkillValue, UI08 skillToSet );

	UI16		GetDeaths( void ) const;		// can we die 4 billion times?!
	SI16		GetGuildNumber( void ) const;
	UI08		GetFlag( void ) const;

	void		SetDeaths( UI16 newVal );
	void		SetFlag( UI08 newValue );
	void		SetFlagRed( void );
	void		SetFlagBlue( void );
	void		SetFlagGray( void );
	void		SetGuildNumber( SI16 newValue );

	SI08		GetFontType( void ) const;
	void		SetFontType( SI08 newType );

				CChar();
	virtual		~CChar();

	CChar *			Dupe( void );
	virtual void	RemoveFromSight( cSocket *mSock = NULL );
	void			SendWornItems( cSocket *s );
	void			Teleport( void );
	void			ExposeToView( void );
	virtual void	Update( cSocket *mSock = NULL );
	virtual void	SendToSocket( cSocket *s );

	CItem *			GetItemAtLayer( UI08 Layer ) const;
	bool			WearItem( CItem *toWear );
	bool			TakeOffItem( UI08 Layer );

	CItem *			FirstItem( void );
	CItem *			NextItem( void );
	bool			FinishedItems( void );

	void			BreakConcentration( cSocket *sock = NULL );

	virtual bool	Save( std::ofstream &outStream );
	virtual void	PostLoadProcessing( void );

	SI16			ActualStrength( void ) const;
	virtual SI16	GetStrength( void ) const;

	SI16			ActualDexterity( void ) const;
	virtual SI16	GetDexterity( void ) const;

	SI16			ActualIntelligence( void ) const;
	virtual SI16	GetIntelligence( void ) const;

	void	IncStrength2( SI16 toAdd = 1 );
	void	IncDexterity2( SI16 toAdd = 1 );
	void	IncIntelligence2( SI16 toAdd = 1 );

	bool	IsMurderer( void ) const;
	bool	IsCriminal( void ) const;
	bool	IsInnocent( void ) const;

	void	StopSpell( void );
	bool	SkillUsed( UI08 skillNum ) const;
	void	SkillUsed( bool value, UI08 skillNum );

	bool	IsPolymorphed( void ) const;
	bool	IsIncognito( void ) const;
	void	IsPolymorphed( bool newValue );
	void	IsIncognito( bool newValue );
	bool	IsJailed( void ) const;

	void			SetMaxHP( UI16 newmaxhp, SI16 newoldstr, RACEID newoldrace );
	void			SetMaxMana( SI16 newmaxmana, SI16 newoldint, RACEID newoldrace );
	void			SetMaxStam( SI16 newmaxstam, SI16 newolddex, RACEID newoldrace );
	virtual UI16	GetMaxHP( void );
	SI16			GetMaxMana( void );
	SI16			GetMaxStam( void );
	virtual void	SetMana( SI16 newValue );
	virtual void	SetHP( SI16 newValue );
	virtual void	SetStamina( SI16 newValue );
	virtual void	SetStrength( SI16 newValue );
	virtual void	SetDexterity( SI16 newValue );
	virtual void	SetIntelligence( SI16 newValue );
	virtual void	SetStrength2( SI16 newValue );
	virtual void	SetDexterity2( SI16 newValue );
	virtual void	SetIntelligence2( SI16 newValue );
	void			IncStamina( SI16 toInc );
	void			IncMana( SI16 toInc );

	void			ToggleCombat( void );

	virtual void	SetPoisoned( UI08 newValue );
		
	bool			isHuman( void );
	bool			inDungeon( void );

	void			talk( cSocket *s, SI32 dictEntry, bool antispam, ... );
	void			talk( cSocket *s, std::string txt, bool antispam );
	void			talkAll( std::string txt, bool antispam );
	void			talkAll( SI32 dictEntry, bool antispam, ... );

	void			emote( cSocket *s, std::string txt, bool antispam );
	void			emote( cSocket *s, SI32 dictEntry, bool antispam, ... );
	void			emoteAll( SI32 dictEntry, bool antispam, ... );

	virtual void	Cleanup( void );
	virtual void	Delete( void );
	virtual bool	CanBeObjType( ObjectType toCompare ) const;
// NPC Characters
protected:
	SI16			npcaitype;
	cBaseObject *	petguarding;
	SI16			taming;
	UI08			trainingplayerin;
	UI32			holdg;

	UI08		split;
	UI08		splitchance;

	SI16		fx[2]; //NPC Wander Point x
	SI16		fy[2]; //NPC Wander Point y
	SI08		fz; //NPC Wander Point z

	SI08		npcWander; // NPC Wander Mode
	SI08		oldnpcWander; // Used for fleeing npcs
	std::queue< UI08 >	pathToFollow;	// let's use a queue of directions to follow

	SI16		spattack;
	SI08		spadelay;	// won't time out for more than 255 seconds!

	UI08		questType;
	UI08		questDestRegion;
	UI08		questOrigRegion;
		
	SI16		fleeat;		// HP Level to flee at
	SI16		reattackat;	// HP Level to re-Attack at

	CHARLIST	petFriends;

	SERIAL		ftarg; // NPC Follow Target

	virtual void	RemoveSelfFromOwner( void );
	virtual void	AddSelfToOwner( void );
public:
	CHARLIST *	GetFriendList( void );

	void		AddFriend( CChar *toAdd );
	void		RemoveFriend( CChar *toRemove );

	SI16		GetNPCAiType( void ) const;
	SI16		GetTaming( void ) const;
	UI08		GetTrainingPlayerIn( void ) const;
	UI32		GetHoldG( void ) const;
	UI08		GetSplit( void ) const;
	UI08		GetSplitChance( void ) const;

	void		SetNPCAiType( SI16 newValue );
	void		SetTaming( SI16 newValue );
	void		SetTrainingPlayerIn( UI08 newValue );
	void		SetHoldG( UI32 newValue );
	void		SetSplit( UI08 newValue );
	void		SetSplitChance( UI08 newValue );

	void			SetGuarding( cBaseObject *newValue );

	cBaseObject *	GetGuarding( void ) const;

	SI16		GetFx( UI08 part ) const;
	SI16		GetFy( UI08 part ) const;
	SI08		GetFz( void ) const;
	SI08		GetNpcWander( void ) const;
	SI08		GetOldNpcWander( void ) const;

	void		SetFx( SI16 newVal, UI08 part );
	void		SetFy( SI16 newVal, UI08 part );
	void		SetFz( SI08 newVal );
	void		SetNpcWander( SI08 newValue );
	void		SetOldNpcWander( SI08 newValue );

	CChar *		GetFTarg( void ) const;
	void		SetFTarg( CChar *newTarg );

	SI16		GetSpAttack( void ) const;
	SI08		GetSpDelay( void ) const;

	void		SetSpAttack( SI16 newValue );
	void		SetSpDelay( SI08 newValue );

	UI08		GetQuestType( void ) const;
	UI08		GetQuestOrigRegion( void ) const;
	UI08		GetQuestDestRegion( void ) const;

	void		SetQuestDestRegion( UI08 newValue );
	void		SetQuestType( UI08 newValue );
	void		SetQuestOrigRegion( UI08 newValue );

	SI16		GetFleeAt( void ) const;
	SI16		GetReattackAt( void ) const;

	void		SetFleeAt( SI16 newValue );
	void		SetReattackAt( SI16 newValue );

	bool		IsValidMount( void ) const;
	bool		IsMounted( void ) const;

	UI08		PopDirection( void );
	void		PushDirection( UI08 newDir );
	bool		StillGotDirs( void ) const;
	void		FlushPath( void );

// Player Characters
protected:
	SERIAL		robe;

	SERIAL		trackingtarget; // Tracking target ID
	CHARLIST	trackingtargets;

	ACCOUNTSBLOCK ourAccount;

	UI16		origID; // Backup of body type for polymorph
	UI16		origSkin;
	std::string	origName; // original name - for Incognito

	UI16		hairstyle;
	UI16		beardstyle;
	COLOUR		haircolor;
	COLOUR		beardcolour;

	std::string	laston; //Last time a character was on

	UI08		commandLevel;		// 0 = player, 1 = counselor, 2 = GM
	UI08		postType;
	SI16		callnum;		// Callnum GM or Counsellor is on
	SI16		playercallnum;	// Players call number in GM or Counsellor Queue

	UI08		squelched; // zippy  - squelching

	CItem *		speechItem;
	UI08		speechMode;
	UI08		speechID;
	cScript *	speechCallback;
	// speechMode valid values
	// 0 normal speech
	// 1 GM page
	// 2 Counselor page
	// 3 Player Vendor item pricing
	// 4 Player Vendor item describing
	// 5 Key renaming
	// 6 Name deed
	// 7 Rune renaming
	// 8 Sign renaming
	// 9 JavaScript speech
public:
	void					SetAccount( ACCOUNTSBLOCK& actbAccount );
	ACCOUNTSBLOCK &			GetAccount(void);
	const ACCOUNTSBLOCK &	GetConstAccount( void ) const;
	UI16					GetAccountNum( void ) const;

	void		SetRobe( SERIAL newValue );
	SERIAL		GetRobe( void ) const;

	UI16		GetOrgID( void ) const;
	void		SetOrgSkin( UI16 value );
	void		SetOrgID( UI16 value );
	UI16		GetOrgSkin( void ) const;
	std::string	GetOrgName( void ) const;
	void		SetOrgName( std::string newName );

	UI08		GetCommandLevel( void ) const;
	void		SetCommandLevel( UI08 newValue );
	UI08		GetPostType( void ) const;
	void		SetPostType( UI08 newValue );
	void		SetPlayerCallNum( SI16 newValue );
	void		SetCallNum( SI16 newValue );

	SI16		GetCallNum( void ) const;
	SI16		GetPlayerCallNum( void ) const;

	void		SetLastOn( std::string newValue );
	std::string GetLastOn( void ) const;

	CChar *		GetTrackingTarget( void ) const;
	CChar *		GetTrackingTargets( UI08 targetNum ) const;
	void		SetTrackingTarget( CChar *newValue );
	void		SetTrackingTargets( CChar *newValue, UI08 targetNum );

	UI08		GetSquelched( void ) const;
	void		SetSquelched( UI08 newValue );

	CItem *		GetSpeechItem( void ) const;
	UI08		GetSpeechMode( void ) const;
	UI08		GetSpeechID( void ) const;
	cScript *	GetSpeechCallback( void ) const;

	void		SetSpeechMode( UI08 newValue );
	void		SetSpeechID( UI08 newValue );
	void		SetSpeechCallback( cScript *newValue );
	void		SetSpeechItem( CItem *newValue );

	UI16		GetHairStyle( void ) const;
	UI16		GetBeardStyle( void ) const;
	COLOUR		GetHairColour( void ) const;
	COLOUR		GetBeardColour( void ) const;

	void		SetHairColour( COLOUR value );
	void		SetBeardColour( COLOUR value );
	void		SetHairStyle( UI16 value );
	void		SetBeardStyle( UI16 value );

};

}

#endif

