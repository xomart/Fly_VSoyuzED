//-------------------------------------------------------------------------------
// ZMSpaceCraft.cpp
// À. Íèêîëåíêî 03.08.2018
//-------------------------------------------------------------------------------
#pragma once
#include <stdafx.h>
#include <FlyCoreSource\\ZMSpaceCraft.h>
#include <FlyCoreSource\\ZCISAManager.h>
#include <FlyCoreSource\\ZMForceAtmSMA81.h>
#include <FlyCoreSource\\ZMForceAtmSMA62.h>
#include <FlyCoreSource\\ZMForceAtmSMAGost440181.h>
#include <FlyCoreSource\\ZMForceAtm.h>
#include <FlyCoreSource\\ZHBL.h>

//-------------------------------------------------------------------------------
ZMSC::ZMSC( ) : ZCIntegrator( ) 
{ 
	memset(&FSC,0, sizeof(FSC)) ;
	memset(&FSP,0, sizeof(FSP)) ;
	memset(&FSE,0, sizeof(FSP)) ;
	memset(&FSR,0, sizeof(FSP)) ;
	memset(&FV, 0, sizeof(FV )) ;
	memset(&KA, 0, sizeof(ZKA)) ;
	memset(&NU, 0, sizeof(ZNU)) ;
	memset(&LSF,0, sizeof(ZLSF)) ;

	Vitok75 = 0 ;

	m_ISAManager   = 0 ;
	m_AtmSMA81     = 0 ;
	m_AtmSMA62     = 0 ;
	m_AtmSMA440181 = 0 ;

	HStoryClarify = 0 ;

	m_RotJ[0] = m_RotJ[1] = m_RotJ[2] = 1.0;
	m_RotJinv[0] = m_RotJinv[1] = m_RotJinv[2] = 1.0;
	m_RotMoment[0] = m_RotMoment[1] = m_RotMoment[2] = 0.0;
	m_RotQuatNormalize = true;

	PreInitInfo.defautFixedStep = 0;
}

//-------------------------------------------------------------------------------
ZMSC::~ZMSC( ) 
{ 
	Clear( ) ;
	m_pNotice = 0 ;
}

//-------------------------------------------------------------------------------
ZSC_VP* HMIN_VI(ZSC_VI& I) 
{
	ZSC_VP* pVP=nullptr ;
	if (I.Hmin.isFind) pVP = &I.Hmin ;
	if (I.Hmin2.isFind) {
		if (I.Hmin2.H<I.Hmin.H) {
			pVP = &I.Hmin2 ;
			if (I.Hmin3.isFind && I.Hmin3.H<I.Hmin2.H) pVP = &I.Hmin3 ;
		} else {
			if (I.Hmin3.isFind && I.Hmin3.H<I.Hmin.H) pVP = &I.Hmin3 ;
	}	}
	return pVP ;
}

ZSC_VP* HMAX_VI(ZSC_VI& I) 
{
	ZSC_VP* pVP=nullptr ;
	if (I.Hmax.isFind) pVP = &I.Hmax ;
	if (I.Hmax2.isFind) {
		if (I.Hmax2.H>I.Hmax.H) {
			pVP = &I.Hmax2 ;
			if (I.Hmax3.isFind && I.Hmax3.H>I.Hmax2.H) pVP = &I.Hmax3 ;
		} else {
			if (I.Hmax3.isFind && I.Hmax3.H>I.Hmax.H) pVP = &I.Hmax3 ;
	}	}
	return pVP ;
}

ZSC_VP* RMIN_VI(ZSC_VI& I) 
{
	ZSC_VP* pVP=nullptr ;
	if (I.Rmin.isFind) pVP = &I.Rmin ;
	if (I.Rmin2.isFind) {
		if (I.Rmin2.H<I.Rmin.H) {
			pVP = &I.Rmin2 ;
			if (I.Rmin3.isFind && I.Rmin3.H<I.Rmin2.H) pVP = &I.Rmin3 ;
		} else {
			if (I.Rmin3.isFind && I.Rmin3.H<I.Rmin.H) pVP = &I.Rmin3 ;
	}	}
	return pVP ;
}

ZSC_VP* RMAX_VI(ZSC_VI& I) 
{
	ZSC_VP* pVP=nullptr ;
	if (I.Rmax.isFind) pVP = &I.Rmax ;
	if (I.Rmax2.isFind) {
		if (I.Rmax2.H>I.Rmax.H) {
			pVP = &I.Rmax2 ;
			if (I.Rmax3.isFind && I.Rmax3.H>I.Rmax2.H) pVP = &I.Rmax3 ;
		} else {
			if (I.Rmax3.isFind && I.Rmax3.H>I.Rmax.H) pVP = &I.Rmax3 ;
	}	}
	return pVP ;
}

//-------------------------------------------------------------------------------
void ZMSC::Clear( ) 
{	
	FreeStory( ) ;
	if (m_ISAManager  ) delete m_ISAManager ; m_ISAManager=0 ;
	if (m_AtmSMA81    ) delete m_AtmSMA81 ; m_AtmSMA81=0 ;
	if (m_AtmSMA62    ) delete m_AtmSMA62 ; m_AtmSMA62=0 ;
	if (m_AtmSMA440181) delete m_AtmSMA440181 ; m_AtmSMA440181=0 ;

	ZCIntegrator::Delete( ) ; 
}

//-------------------------------------------------------------------------------
int ZMSC::Init(ZKEYNU& NuKey, ZCExternalEquationsDefinition* EED) 
{
	ClearErrorsMSG();
	int rc = FlyGlobalInit(); if (rc) return rc;

	ZNU nu;
	// Çàãðóæàåì ÍÓ (èç ôàéëà èëè ÁÄ èëè ..... 
	rc = LoadNU(NuKey, nu); if (rc) return rc;
	// Èíèöèàëèçèðóåì ÌÄ - çàäàåì íà÷àëüíîå çíà÷åíèå âåêòîðà (èç ÍÓ)
	// âûäåëÿåì ïàìÿòü, çàãðóæàåì ËÙÑ, ïàðàìåòðû ÃÏÇ, ïàðàìåòðû ìîäåëè àòìîñôåðû è ò.ä.
	// Ïîñëå ýòîãî ÌÄ ãîòîâà ê èíòåãðèðîâàíèþ ÑÄÓ
	rc = Init(nu, nullptr, EED); if (rc) return rc;
	return 0 ;
}

//-------------------------------------------------------------------------------
int ZMSC::Init(ZNU& nu_, ZLSF* lsf_, ZCExternalEquationsDefinition* EED) 
{
	int rc ;
	NU = nu_ ;

	if (EED) {
		ExtEqDef =*EED ;
		SetExternalEquationsCount(ExtEqDef.Size()) ;
	}

	// Ïåðåâîäèì ÍÓ â ÑÊ èíòåãðèðîâàíèÿ ÑÄÓ
	rc = ConvertSkNu(NU, _SK_ASK1975) ; 
	if (rc) { AddErrorMSG( "Îøèáêà ïðåîáðàçîâàíèÿ çàäàííûõ ÍÓ â ÀÑÊ 1975 ãîäà") ; goto end ; }
	// Çàãðóçêà èíôîðìàöèè î ÊÀ
	if (KA.num!=NU.Key.ka) {
		rc = LoadKA( NU.Key.ka, KA) ; if (rc) goto end ; ;
		if (PreInitInfo.defautFixedStep > 0) {
			KA.min_step = KA.max_step = PreInitInfo.defautFixedStep;
		}
	}
	// Çàãðóçêà äàííûõ ëîãè÷åñêîé øêàëû ñèë
	if (lsf_) {
		memcpy(&LSF, lsf_, sizeof(LSF)); 
		NU.numlsf = LSF.num ; 
	} else {
		if (NU.numlsf!=LSF.num) {
			rc = LoadLSF(NU.numlsf, LSF) ; if ( rc ) goto end ;
		}
	}

	//---------------------------------------------------------------------------
	// Ñîçäàíèå îáúåêòîâ àòìîñôåðû è çàãðóçêà ïàðàìåòðîâ ìîäåëè àòìîñôåðû 
	// Ïðè íåîáõîäèìîñòè ñîçäàíèå äèñïåò÷åðà èíäåêñîâ ÑÀ
	if (LSF.vatm>=ATMDMAGOST1977 && LSF.vatm<=ATMMSIS2000) {
		if (!m_ISAManager) m_ISAManager = new ZCISAManager ; 
		if (!m_ISAManager) { rc = 1 ; goto end ; }
	}

	// Ïðè íåîáúîäèìîñòè ñîçäà¸òñÿ îáúåêò ðàññ÷¸òà ïàðàìåòðîâ ñòàòè÷åñêîé àòìîñôåðû
	if (LSF.vatm==ATMSMA81 ) { 
		if(!m_AtmSMA81 ) m_AtmSMA81 = new ZMAtmSMA81 ; 
		if (!m_AtmSMA81) { rc = 1 ; goto end ; } 
	}
	if (LSF.vatm==ATMSMA62 ) { 
		if(!m_AtmSMA62 ) m_AtmSMA62 = new ZMAtmSMA62 ; 
		if (!m_AtmSMA62) { rc = 1 ; goto end ; } 
	}
	if (LSF.vatm==ATMSMA440181) { 
		if(!m_AtmSMA440181) m_AtmSMA440181 = new ZMAtmSMA440181 ; 
		if (!m_AtmSMA440181) { rc = 1 ; goto end ; } 
	}

	// Çàãðóçêà ìîäåëè ãðàâèòàöèîííîãî ïîëÿ Çåìëè
	rc = LoadGravPole(&GPZ, LSF.vgpz, LSF.ngpz, LSF.mgpz) ; if (rc) goto end ;

	IntegratorCreateStruct cs ;
	// Èíòåãðèðóåì ïàðàìåòðû t, X, Y, Z, Vx, Vy, Vz, m - âñåãî 8 ïàðàìåòðîâ
	cs.n          = 8 ;
	// Ìåòîä èíòåãðèðîâàíèÿ ÐÊ 8-ãî ïîðÿäêà
	cs.RungeType  = ID_RUNGE_8 ;
	cs.AdamsType  = 0 ; //ID_ADAMS_8 ;
	cs.step_min   = KA.min_step / k_cbc ;
	cs.step_max   = KA.max_step / k_cbc ;
	cs.step       = cs.step_min ;
	cs.fixed_step = PreInitInfo.defautFixedStep / k_cbc;
	cs.step_eps   = 1e-13 ;
	// Ñ àâòîìàòè÷åñêèì âûáîðîì øàãà
	cs.flag       = F_RUNA_AUTOSTEP ;
	cs.data       = 0 ;
	cs.sph        = 0 ;
	cs.user_break = 0 ;
	cs.arg_index  = 0 ;
	rc = Create(cs) ; if (!rc) { AddErrorMSG( "Îøèáêà ñîçäàíèÿ èíòåãðàòîðà" ) ; return rc ; }

	Vitok75 = NU.vit ;
	// Âûêëàäûâàåì âåêòîð â èíòåãðàòîð
	rc = ZCIntegrator::SetVector(NU.t, NU.X, WeightFullNU()) ; if (rc) goto end ;
	// Ðàñ÷åò âñåõ êèíåìàòè÷åñêèõ ïàðàìåòðîâ è îñêóëèðóþùèõ ýëåìåíòîâ îðáèòû
	// â íà÷àëüíîé òî÷êå - â òî÷êå ÍÓ
	StepCalc(0) ;
	// Èíèöèàöèÿ ñîîáùåíèÿ îá èçìåíåíèè èíòåãðèðóåìîãî âåêòîðà 
	rc = OnMessage(MMSG_SET_VECTOR, &(*this)[0], &((*this)[1])) ; if (rc) goto end ;

end : ;
	if (rc) { AddErrorMSG( "Îøèáêà èíèöèàëèçàöèè ìîäåëè äâèæåíèÿ ÊÀ" ) ; return rc ; }
	return 0 ;
}

//-------------------------------------------------------------------------------
double ZMSC::GetTime() const
{
	const ZSC_SI* FS = &(FSR.E.ID==ModEvent_EMPTY ? FSC:FSR) ;
	return(FS->ta) ;
}

//-------------------------------------------------------------------------------
long ZMSC::GetVitok(SK_TYPE sk, ZSC_SI* F) const
{
	const ZSC_SI* FS = F? F:&(FSR.E.ID==ModEvent_EMPTY ? FSC:FSR) ;
	switch(sk) {
		case _SK_ASK1975: return FS->Vit75 ; break ;
		case _SK_ASK2000: return FS->Vit00 ; break ;
		case _SK_ASKTE  : return FS->VitTE ; break ;
		case _SK_ASKTES : return FS->VitTES ; break ;
	}
	return 0 ;
}

//-------------------------------------------------------------------------------
double ZMSC::GetU(SK_TYPE sk, ZSC_SI* F, bool isbpl) const
{
	const ZSC_SI* FS = F? F:&(FSR.E.ID==ModEvent_EMPTY ? FSC:FSR) ;
	switch(sk) {
		case _SK_ASK1975: return(isbpl?FS->OSK1975bpl[5]:FS->OSK1975[5]) ; break ;
		case _SK_ASK2000: return(isbpl?FS->OSK2000bpl[5]:FS->OSK2000[5]) ; break ;
		case _SK_ASKTE  : return(isbpl?FS->OSKTEbpl[5]  :FS->OSKTE[5]) ; break ;
		case _SK_ASKTES : return(isbpl?FS->OSKTESbpl[5] :FS->OSKTES[5]) ; break ;
	}
	return 0 ;
}

//-------------------------------------------------------------------------------
void ZMSC::GetKA(ZKA& ka) const { ka = KA; }
//-------------------------------------------------------------------------------
void ZMSC::GetNU(ZNU& nu) const { nu = NU; }
//-------------------------------------------------------------------------------
int ZMSC::SetNU(ZNU& nu)
{ 
	NU = nu ; 	
	SetVector(NU.X, NU.vit, NU.t, WeightFullNU(), NU.sk) ; 
	return 0 ; 
}  

//-------------------------------------------------------------------------------
double ZMSC::WeightNU()     const {return(NU.M0>0 ? NU.M0:KA.M0);}
double ZMSC::WeightFullNU() const {return(WeightNU() + (NU.MF>0 ? NU.MF:KA.MF));}
double ZMSC::WeightFuel()   const {return (m_a[7] - (NU.M0>0 ? NU.M0:KA.M0));}
double ZMSC::WeightFull()   const {return m_a[7];}

//-------------------------------------------------------------------------------
void ZMSC::GetLSF(ZLSF& lsf) const {lsf = LSF;}

//-------------------------------------------------------------------------------
int ZMSC::SetVector(double* X, long Vit, double t, double m, SK_TYPE sk) 
{
	double X75[7], T[7] ;
	int    rc = 0 ;

	X75[0] = t ;
	if (sk==_SK_ASK1975 || sk==_SK_OSK1975) {
		if (sk==_SK_OSK1975) ZOSCtoASC(X, &X75[1]) ;
		else memcpy(&X75[1], X, 6*sizeof(double)) ;
		Vitok75 = Vit ;		
	}
	if (sk==_SK_ASK2000 || sk==_SK_OSK2000) {
		if (sk==_SK_OSK2000) ZOSCtoASC(X, T) ;
		else memcpy(T, X, 6*sizeof(double)) ;
		rc = ZASC_2000to1975(T, &X75[1], 1) ; RRC ;
		TestEpsVit(sk, X, Vit, _SK_ASK1975, &X75[1], Vitok75) ;
		//Vitok75 = VitSK(T, Vit, &X75[1]) ;
	}
	if (sk==_SK_ASKTE || sk==_SK_OSKTE) {
		T[0] = t ;
		if (sk==_SK_OSKTE ) ZOSCtoASC(X, &T[1]) ;
		else memcpy(&T[1], X, 6*sizeof(double)) ;
		ZASC_TEto1975(t, &T[1], &X75[1], 1) ;
		TestEpsVit(sk, X, Vit, _SK_ASK1975, &X75[1], Vitok75) ;
		//Vitok75 = VitSK(&T[1], Vit, &X75[1]) ;
	}
	if (sk==_SK_ASKTES || sk==_SK_OSKTES) {
		T[0] = t ;
		if (sk==_SK_OSKTE) ZOSCtoASC(X, &T[1]) ;
		else memcpy(&T[1], X, 6*sizeof(double)) ;
		ZASC_TESto1975(t, &T[1], &X75[1], 1) ;
		TestEpsVit(sk, X, Vit, _SK_ASK1975, &X75[1], Vitok75) ;
		//Vitok75 = VitSK(&T[1], Vit, &X75[1]) ;
	}
	if (sk==_SK_GSK) {
		rc = ZGSCtoASC1975(t, X, &X75[1], 0) ; 
		TestEpsVit(sk, X, Vit, _SK_ASK1975, &X75[1], Vitok75) ;
		//Vitok75 = VitSK(X, Vit, &X75[1]) ;
	}
	rc = ZCIntegrator::SetVector(X75[0], &X75[1], m) ; RRC ;
	StepCalc(0) ;
	rc = OnMessage(MMSG_SET_VECTOR, &(*this)[0], &((*this)[1])) ; RRC ;
	
	return rc ; 
}

//-------------------------------------------------------------------------------
int ZMSC::GetVector(double* pX, long* Vit, 
					double* pt, double* pm, SK_TYPE sk) const
{
	int rc = 0 ;
	const ZSC_SI* FS = &(FSR.E.ID==ModEvent_EMPTY ? FSC:FSR) ;

	if (pt) *pt = FS->ta ;
	if (pm) *pm = FS->m ;

	switch(sk) { 
	case _SK_ASK1975:
		if (Vit) *Vit = FS->Vit75 ;
		if (pX ) memcpy( pX, FS->ASK1975, 6*sizeof(double)) ;
		break ;
	case _SK_OSK1975:
		if (Vit) *Vit = FS->Vit75 ;
		if (pX ) memcpy( pX, FS->OSK1975, 6*sizeof( double ) ) ;
		break ;
	case _SK_GSK:
		if (Vit) *Vit = FS->VitTE ;
		if (pX ) memcpy( pX, FS->GSK, 6*sizeof( double ) ) ;
		break ;
	case _SK_ASK2000:
		if (Vit) *Vit = FS->Vit00 ;
		if (pX ) memcpy( pX, FS->ASK2000, 6*sizeof( double ) ) ;
		break ;
	case _SK_OSK2000:
		if (Vit) *Vit = FS->Vit00 ;
		if (pX ) memcpy( pX, FS->OSK2000, 6*sizeof( double ) ) ;
		break ;
	case _SK_ASKTE:
		if (Vit) *Vit = FS->VitTE ;
		if (pX ) memcpy( pX, FS->ASKTE, 6*sizeof( double ) ) ;
		break ;
	case _SK_OSKTE:
		if (Vit) *Vit = FS->VitTE ;
		if (pX ) memcpy( pX, FS->OSKTE, 6*sizeof( double ) ) ;
		break ;
	case _SK_ASKTES:
		if (Vit) *Vit = FS->VitTES ;
		if (pX ) memcpy( pX, FS->ASKTES, 6*sizeof( double ) ) ;
		break ;
	case _SK_OSKTES:
		if (Vit) *Vit = FS->VitTES ;
		if (pX ) memcpy( pX, FS->OSKTES, 6*sizeof( double ) ) ;
		break ;		
	}
	return rc ;
}

//-------------------------------------------------------------------------------
int ZMSC::StepBreak( double t, double* X ) 
{
	return 0 ;
}

//-------------------------------------------------------------------------------
int ZMSC::OnMessage( long ModMsgID, double* t, double* X) 
{
	//if ( ModMsgID == MMSG_TDU_ON     && SignalOnStartDU ) { SignalOnStartDU( this, MMSG_TDU_ON, ( void* )&t, ( void* )X ) ; return 0 ; } 
	//if ( ModMsgID == MMSG_TDU_OFF    && SignalOnEndDU   ) { SignalOnEndDU  ( this, MMSG_TDU_OFF, ( void* )&t, ( void* )X ) ; return 0 ; }
	//if ( ModMsgID == MMSG_SET_VECTOR && SignalOnSetVec  ) { SignalOnSetVec ( this, MMSG_SET_VECTOR, ( void* )&t, ( void* )X ) ; return 0 ; } 
	//if ( ModMsgID == MMSG_RESTORY    && SignalOnRestory ) { SignalOnRestory( this, MMSG_RESTORY, ( void* )&t, ( void* )X ) ; return 0 ; }
	//if ( ModMsgID == MMSG_START      && SignalOnStart   ) { SignalOnStart  ( this, MMSG_START, ( void* )&t, ( void* )X ) ; return 0 ; } 
	//if ( ModMsgID == MMSG_END        && SignalOnEnd     ) { SignalOnEnd    ( this, MMSG_END, ( void* )&t, ( void* )X ) ; return 0 ; }
	// Normalize quaternion for rotational external equations to suppress drift.
	if (m_RotQuatNormalize && X && GetExternalEquationsCount()>0) {
		double* Xeq = X + 7;
		int iQ0=-1, iQ1=-1, iQ2=-1, iQ3=-1;
		for (int i=0; i<ExtEqDef.Size(); ++i) {
			const int id = ExtEqDef.ID[i];
			if (id==EXTEQ_ID_rot_q0) iQ0=i;
			else if (id==EXTEQ_ID_rot_q1) iQ1=i;
			else if (id==EXTEQ_ID_rot_q2) iQ2=i;
			else if (id==EXTEQ_ID_rot_q3) iQ3=i;
		}
		if (iQ0>=0 && iQ1>=0 && iQ2>=0 && iQ3>=0) {
			double q0=Xeq[iQ0], q1=Xeq[iQ1], q2=Xeq[iQ2], q3=Xeq[iQ3];
			double nq = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
			if (nq>1e-20) {
				Xeq[iQ0]=q0/nq; Xeq[iQ1]=q1/nq; Xeq[iQ2]=q2/nq; Xeq[iQ3]=q3/nq;
			}
		}
	}
	return 0 ;
}

//-------------------------------------------------------------------------------
int ZMSC::OnEvent(ZMOD_EVENT& E, ZSC_SI* FSCE) 
{
	if (IsVITEvent(E.ID)) return OnVitokEvent(E, FSCE) ;
	if (IsZRVEvent(E.ID)) return OnZRVEvent(E, FSCE) ;
	if (IsUserEvent(E.ID)) return OnUserEvent(E, FSCE) ;
	return 0 ;
}

//-------------------------------------------------------------------------------
int ZMSC::OnFullZRV(ZZRV* ZRV, ZSC_SI* FSC) 
{
	return 0 ;
}

//-------------------------------------------------------------------------------
void ZMSC::ASKtoASKbpl(double* X, double* Xbpl) 
{
	// Áàçîâàÿ ïëîñêîñòü 0 - ýêâàòîð, 1 - XOZ, 2 - YOZ	
	if (KA.bpl==0) {
		memcpy(Xbpl, X, 6*sizeof(double)) ;
		return ;
	}
	if (KA.bpl==1) {
		// Z -> X,  X -> Y,  Y -> Z
		Xbpl[0]=X[2] ; Xbpl[1]=X[0] ; Xbpl[2]=X[1] ;
		Xbpl[3]=X[5] ; Xbpl[4]=X[3] ; Xbpl[5]=X[4] ;
		return ;
	}
	if ( KA.bpl == 2 ) {
		// Z -> X,  Y -> Y,  X -> -Z
		Xbpl[0]=X[2] ; Xbpl[1]=X[1] ; Xbpl[2]=-X[0] ;
		Xbpl[3]=X[5] ; Xbpl[4]=X[4] ; Xbpl[5]=-X[3] ;
		return ;
	}
}

//-------------------------------------------------------------------------------
void ZMSC::ExternalEquationsOn (double* NU) 
{
//-------------------------------------------------------------------------------
void ZMSC::SetRotInertiaDiag(double Jx, double Jy, double Jz)
{
	m_RotJ[0] = Jx; m_RotJ[1] = Jy; m_RotJ[2] = Jz;
	m_RotJinv[0] = (fabs(Jx)>1e-20 ? 1.0/Jx : 0.0);
	m_RotJinv[1] = (fabs(Jy)>1e-20 ? 1.0/Jy : 0.0);
	m_RotJinv[2] = (fabs(Jz)>1e-20 ? 1.0/Jz : 0.0);
}

//-------------------------------------------------------------------------------
void ZMSC::SetRotConstantMoment(double Mx, double My, double Mz)
{
	m_RotMoment[0] = Mx; m_RotMoment[1] = My; m_RotMoment[2] = Mz;
}

//-------------------------------------------------------------------------------
void ZMSC::SetRotQuatNormalization(bool On)
{
	m_RotQuatNormalize = On;
}

//-------------------------------------------------------------------------------
void ZMSC::FillRotExternalDefinition(ZCExternalEquationsDefinition& EED) const
{
	EED << EXTEQ_ID_rot_q0;
	EED << EXTEQ_ID_rot_q1;
	EED << EXTEQ_ID_rot_q2;
	EED << EXTEQ_ID_rot_q3;
	EED << EXTEQ_ID_rot_wx;
	EED << EXTEQ_ID_rot_wy;
	EED << EXTEQ_ID_rot_wz;
}

//-------------------------------------------------------------------------------
void ZMSC::FillRotInitialState(double* Xrot, const double* q, const double* w, bool Normalize) const
{
	if (!Xrot) return;
	Xrot[0] = (q ? q[0] : 1.0);
	Xrot[1] = (q ? q[1] : 0.0);
	Xrot[2] = (q ? q[2] : 0.0);
	Xrot[3] = (q ? q[3] : 0.0);
	Xrot[4] = (w ? w[0] : 0.0);
	Xrot[5] = (w ? w[1] : 0.0);
	Xrot[6] = (w ? w[2] : 0.0);
	if (Normalize) {
		double nq = sqrt(Xrot[0]*Xrot[0] + Xrot[1]*Xrot[1] + Xrot[2]*Xrot[2] + Xrot[3]*Xrot[3]);
		if (nq>1e-20) {
			Xrot[0]/=nq; Xrot[1]/=nq; Xrot[2]/=nq; Xrot[3]/=nq;
		}
	}
}

	int iQ0=-1, iQ1=-1, iQ2=-1, iQ3=-1, iWx=-1, iWy=-1, iWz=-1;
	for (i=0; i<N; ++i) {
		switch(ExtEqDef.ID[i]) {
			case EXTEQ_ID_rot_q0: iQ0=i; break;
			case EXTEQ_ID_rot_q1: iQ1=i; break;
			case EXTEQ_ID_rot_q2: iQ2=i; break;
			case EXTEQ_ID_rot_q3: iQ3=i; break;
			case EXTEQ_ID_rot_wx: iWx=i; break;
			case EXTEQ_ID_rot_wy: iWy=i; break;
			case EXTEQ_ID_rot_wz: iWz=i; break;
		}
	}

	double rot_q0_dot=0.0, rot_q1_dot=0.0, rot_q2_dot=0.0, rot_q3_dot=0.0;
	double rot_wx_dot=0.0, rot_wy_dot=0.0, rot_wz_dot=0.0;
	bool hasRot = (iQ0>=0 && iQ1>=0 && iQ2>=0 && iQ3>=0 && iWx>=0 && iWy>=0 && iWz>=0);
	if (hasRot && Xexe) {
		double q0 = Xexe[iQ0], q1 = Xexe[iQ1], q2 = Xexe[iQ2], q3 = Xexe[iQ3];
		double wx = Xexe[iWx], wy = Xexe[iWy], wz = Xexe[iWz];
		rot_q0_dot =-0.5*(q1*wx + q2*wy + q3*wz);
		rot_q1_dot = 0.5*(q0*wx + q2*wz - q3*wy);
		rot_q2_dot = 0.5*(q0*wy + q3*wx - q1*wz);
		rot_q3_dot = 0.5*(q0*wz + q1*wy - q2*wx);

		double Jw_x = m_RotJ[0]*wx;
		double Jw_y = m_RotJ[1]*wy;
		double Jw_z = m_RotJ[2]*wz;
		double c_x = wy*Jw_z - wz*Jw_y;
		double c_y = wz*Jw_x - wx*Jw_z;
		double c_z = wx*Jw_y - wy*Jw_x;
		rot_wx_dot = m_RotJinv[0]*(m_RotMoment[0] - c_x);
		rot_wy_dot = m_RotJinv[1]*(m_RotMoment[1] - c_y);
		rot_wz_dot = m_RotJinv[2]*(m_RotMoment[2] - c_z);
	}

		case EXTEQ_ID_rot_q0:
			Yexe[i] = rot_q0_dot;
			break;
		case EXTEQ_ID_rot_q1:
			Yexe[i] = rot_q1_dot;
			break;
		case EXTEQ_ID_rot_q2:
			Yexe[i] = rot_q2_dot;
			break;
		case EXTEQ_ID_rot_q3:
			Yexe[i] = rot_q3_dot;
			break;
		case EXTEQ_ID_rot_wx:
			Yexe[i] = rot_wx_dot;
			break;
		case EXTEQ_ID_rot_wy:
			Yexe[i] = rot_wy_dot;
			break;
		case EXTEQ_ID_rot_wz:
			Yexe[i] = rot_wz_dot;
			break;
	// "Âêëþ÷àåì" âíåøíèå óðàâíåíèÿ
	ZCIntegrator::ExternalEquationsOn(NU) ;
	// Êîïèðóåì ÍÓ äëÿ âíåøíèõ óðàâíåíèé â êîíåö èíòåãðèðóåìîãî âåêòîðà
	ZSC_SI* SI = &FSC ;
	SI->ExternalEqCount = GetExternalEquationsCount() ;
	SI->ExternalEqOn = true ;
	if (NU)	memcpy(&SI->ExternalEq, NU, GetExternalEquationsCount()*sizeof(double)) ;
	else memset(&SI->ExternalEq, 0, GetExternalEquationsCount()*sizeof(double)) ;
}

//-------------------------------------------------------------------------------
void ZMSC::ExternalEquationsOff(double* X) 
{
	// "Âêëþ÷àåì" âíåøíèå óðàâíåíèÿ
	ZCExternalEquations::ExternalEquationsOff(X) ;
	// Êîïèðóåì ÍÓ äëÿ âíåøíèõ óðàâíåíèé â êîíåö èíòåãðèðóåìîãî âåêòîðà
	ZSC_SI* SI = &(FSR.E.ID==ModEvent_EMPTY ? FSC:FSR) ;
	SI->ExternalEqOn = false ;
	if (X)	memcpy(X, &SI->ExternalEq, GetExternalEquationsCount()*sizeof(double)) ;
}

//-------------------------------------------------------------------------------
int ZMSC::ExternalSph(double t, double* mainX, double* Xexe, double* Yexe) 
{
	int i, N = ExtEqDef.Size() ;
	if (!N) return 0 ;

	int rc = 0 ;
	double ASC_TE[6], OE_TE[6] ;
	rc = ZASC_1975toTE(t, mainX, ASC_TE, true) ; RRC ;
	ZASCtoOSC(ASC_TE, OE_TE) ;

	for (i=0; i<N; i++) {
		unsigned int IDExt = ExtEqDef.ID[i] ;
		switch(IDExt) { 
		case EXTEQ_ID_avrT_H:
			double GSK[6] ;
			rc = ZASC1975toGSC(t, mainX, GSK, 0) ;	RRC ;
			double HBL[3] ;
			ZGRtoHBL(GSK, HBL, 0, 0) ;
			Yexe[i] = 1e4*HBL[0] ;
			break ;
		case EXTEQ_ID_avrT_ex:
			Yexe[i] = OE_TE[1]*cos(OE_TE[4]) ;
			break ;
		case EXTEQ_ID_avrT_ey:
			Yexe[i] = OE_TE[1]*sin(OE_TE[4]) ;
			break ;
		case EXTEQ_ID_avrT_ix:
			Yexe[i] = tan(OE_TE[2]/2.0)*cos(OE_TE[3]) ;
			break ;
		case EXTEQ_ID_avrT_iy:
			Yexe[i] = tan(OE_TE[2]/2.0)*sin(OE_TE[3]) ;
			break ;
		}
	}
	return 0 ;
}

//-------------------------------------------------------------------------------