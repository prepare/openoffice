/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/frame/XTitle.hpp>

#include <tools/list.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/srchitem.hxx>
#include <svl/macitem.hxx>
#include <gloshdl.hxx>

#include <editeng/acorrcfg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <uitool.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <glshell.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <initui.hxx>					// fuer ::GetGlossaries()
#include <cmdid.h>
#include <swerror.h>
#include <misc.hrc>


#define SwWebGlosDocShell
#define SwGlosDocShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>

using namespace ::com::sun::star;

SFX_IMPL_INTERFACE( SwGlosDocShell, SwDocShell, SW_RES(0) )
{
}

SFX_IMPL_INTERFACE( SwWebGlosDocShell, SwWebDocShell, SW_RES(0) )
{
}


TYPEINIT1( SwGlosDocShell, SwDocShell );
TYPEINIT1( SwWebGlosDocShell, SwWebDocShell );


void lcl_Execute( SwDocShell& rSh, SfxRequest& rReq )
{
	if ( rReq.GetSlot() == SID_SAVEDOC )
	{
		if( !rSh.HasName() )
		{
			rReq.SetReturnValue( SfxBoolItem( 0, rSh.Save() ) );
		}
		else
		{
			const SfxBoolItem* pRes = ( const SfxBoolItem* )
										rSh.ExecuteSlot( rReq,
										rSh.SfxObjectShell::GetInterface() );
			if( pRes->GetValue() )
				rSh.GetDoc()->ResetModified();
		}
	}
}


void lcl_GetState( SwDocShell& rSh, SfxItemSet& rSet )
{
	if( SFX_ITEM_AVAILABLE >= rSet.GetItemState( SID_SAVEDOC, sal_False ))
	{
		if( !rSh.GetDoc()->IsModified() )
			rSet.DisableItem( SID_SAVEDOC );
		else
			rSet.Put( SfxStringItem( SID_SAVEDOC, SW_RESSTR(STR_SAVE_GLOSSARY)));
	}
}


sal_Bool lcl_Save( SwWrtShell& rSh, const String& rGroupName,
				const String& rShortNm, const String& rLongNm )
{
	const SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
	SwTextBlocks * pBlock = ::GetGlossaries()->GetGroupDoc( rGroupName );

	SvxMacro aStart(aEmptyStr, aEmptyStr);
	SvxMacro aEnd(aEmptyStr, aEmptyStr);
	SwGlossaryHdl* pGlosHdl;

	pGlosHdl = rSh.GetView().GetGlosHdl();
	pGlosHdl->GetMacros( rShortNm, aStart, aEnd, pBlock );

	sal_uInt16 nRet = rSh.SaveGlossaryDoc( *pBlock, rLongNm, rShortNm,
                                pCfg->IsSaveRelFile(),
								pBlock->IsOnlyTextBlock( rShortNm ) );

	if(aStart.GetMacName().Len() || aEnd.GetMacName().Len() )
	{
		SvxMacro* pStart = aStart.GetMacName().Len() ? &aStart : 0;
		SvxMacro* pEnd = aEnd.GetMacName().Len() ? &aEnd : 0;
		pGlosHdl->SetMacros( rShortNm, pStart, pEnd, pBlock );
	}

	rSh.EnterStdMode();
	if( USHRT_MAX != nRet )
		rSh.ResetModified();
	delete pBlock;
	return nRet != USHRT_MAX;
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


SwGlosDocShell::SwGlosDocShell(sal_Bool bNewShow)
    : SwDocShell( (bNewShow)
            ? SFX_CREATE_MODE_STANDARD : SFX_CREATE_MODE_INTERNAL )
    , bShow ( bNewShow )
{
	SetHelpId(SW_GLOSDOCSHELL);
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


SwGlosDocShell::~SwGlosDocShell(  )
{
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


void SwGlosDocShell::Execute( SfxRequest& rReq )
{
	::lcl_Execute( *this, rReq );
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


void SwGlosDocShell::GetState( SfxItemSet& rSet )
{
	::lcl_GetState( *this, rSet );
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


sal_Bool SwGlosDocShell::Save()
{
	// In case of an API object which holds this document, it is possible that the WrtShell is already
	// dead. For instance, if the doc is modified via this API object, and then, upon office shutdown,
	// the document's view is closed (by the SFX framework) _before_ the API object is release and
	// tries to save the doc, again.
	// 96380 - 2002-03-03 - fs@openoffice.org
	if ( GetWrtShell() )
		return ::lcl_Save( *GetWrtShell(), aGroupName, aShortName, aLongName );
	else
	{
		SetModified( sal_False );
		return sal_False;
	}
}


/**/

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


SwWebGlosDocShell::SwWebGlosDocShell()
	: SwWebDocShell( SFX_CREATE_MODE_STANDARD )
{
	SetHelpId(SW_WEBGLOSDOCSHELL);
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


SwWebGlosDocShell::~SwWebGlosDocShell(  )
{
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


void SwWebGlosDocShell::Execute( SfxRequest& rReq )
{
	::lcl_Execute( *this, rReq );
}
/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


void SwWebGlosDocShell::GetState( SfxItemSet& rSet )
{
	::lcl_GetState( *this, rSet );
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


sal_Bool SwWebGlosDocShell::Save()
{
	// same comment as in SwGlosDocShell::Save - see there
	if ( GetWrtShell() )
		return ::lcl_Save( *GetWrtShell(), aGroupName, aShortName, aLongName );
	else
	{
		SetModified( sal_False );
		return sal_False;
	}
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

SV_IMPL_REF ( SwDocShell )

SwDocShellRef SwGlossaries::EditGroupDoc( const String& rGroup, const String& rShortName, sal_Bool bShow )
{
	SwDocShellRef xDocSh;

	SwTextBlocks* pGroup = GetGroupDoc( rGroup );
	if( pGroup && pGroup->GetCount() )
	{
		// erfrage welche View registriert ist. Im WebWriter gibts es keine
		// normale View
		sal_uInt16 nViewId = 0 != &SwView::Factory() ? 2 : 6;
		String sLongName = pGroup->GetLongName(pGroup->GetIndex( rShortName ));

		if( 6 == nViewId )
		{
			SwWebGlosDocShell* pDocSh = new SwWebGlosDocShell();
			xDocSh = pDocSh;
			pDocSh->DoInitNew( 0 );
			pDocSh->SetLongName( sLongName );
			pDocSh->SetShortName( rShortName);
			pDocSh->SetGroupName( rGroup );
		}
		else
		{
			SwGlosDocShell* pDocSh = new SwGlosDocShell(bShow);
			xDocSh = pDocSh;
			pDocSh->DoInitNew( 0 );
			pDocSh->SetLongName( sLongName );
			pDocSh->SetShortName( rShortName );
			pDocSh->SetGroupName( rGroup );
		}

		// Dokumenttitel setzen
        SfxViewFrame* pFrame = bShow ? SfxViewFrame::LoadDocument( *xDocSh, nViewId ) : SfxViewFrame::LoadHiddenDocument( *xDocSh, nViewId );
		String aDocTitle(SW_RES( STR_GLOSSARY ));
        aDocTitle += ' ';
        aDocTitle += sLongName;

        bool const bDoesUndo =
            xDocSh->GetDoc()->GetIDocumentUndoRedo().DoesUndo();
        xDocSh->GetDoc()->GetIDocumentUndoRedo().DoUndo( false );

		xDocSh->GetWrtShell()->InsertGlossary( *pGroup, rShortName );
        if( !xDocSh->GetDoc()->getPrinter( false ) )
		{
			// wir erzeugen einen default SfxPrinter.
			// Das ItemSet wird vom Sfx geloescht!
			SfxItemSet *pSet = new SfxItemSet( xDocSh->GetDoc()->GetAttrPool(),
						FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
						SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
						SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
						0 );
			SfxPrinter* pPrinter = new SfxPrinter( pSet );

			// und haengen ihn ans Dokument.
            xDocSh->GetDoc()->setPrinter( pPrinter, true, true );
		}

		xDocSh->SetTitle( aDocTitle );
        try
        {
            // set the UI-title
            uno::Reference< frame::XTitle > xTitle( xDocSh->GetModel(), uno::UNO_QUERY_THROW );
            xTitle->setTitle( aDocTitle );
        }
        catch( uno::Exception& )
        {}

        xDocSh->GetDoc()->GetIDocumentUndoRedo().DoUndo( bDoesUndo );
		xDocSh->GetDoc()->ResetModified();
		if ( bShow )
			pFrame->GetFrame().Appear();

		delete pGroup;
	}
	return xDocSh;
}


