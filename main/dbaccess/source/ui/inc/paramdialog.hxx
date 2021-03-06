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



#ifndef _DBAUI_PARAMDIALOG_HXX_
#define _DBAUI_PARAMDIALOG_HXX_

#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif

#ifndef _DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef CONNECTIVITY_PREDICATEINPUT_HXX
#include <connectivity/predicateinput.hxx>
#endif
#ifndef SVX_QUERYDESIGNCONTEXT_HXX
#include "svx/ParseContext.hxx"
#endif

namespace connectivity
{
	class OSQLParseNode;
}

//.........................................................................
namespace dbaui
{
//.........................................................................

	//==================================================================
	//= OParameterDialog
	//==================================================================
	class OParameterDialog
			:public ModalDialog
			,public ::svxform::OParseContextClient
	{
	protected:
		// the controls
		FixedLine		m_aNamesFrame;
		ListBox			m_aAllParams;
		FixedText		m_aValueFrame;
		Edit			m_aParam;
		PushButton		m_aTravelNext;
		OKButton		m_aOKBtn;
		CancelButton	m_aCancelBtn;

		sal_uInt16			m_nCurrentlySelected;

		::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
						m_xParams;
		::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
						m_xConnection;
		::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >
						m_xFormatter;
		::dbtools::OPredicateInputController
						m_aPredicateInput;

        ByteVector      m_aVisitedParams;
		Timer		m_aResetVisitFlag;
			// we reset the "visited flag" 1 second after and entry has been selected

		sal_Bool	m_bNeedErrorOnCurrent;

		::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
					m_aFinalValues;		/// the final values as entered by the user

	public:
		OParameterDialog(Window* _pParent,
			const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & _rParamContainer,
			const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > & _rxConnection,
			const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
		~OParameterDialog();

		::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
					getValues() const { return m_aFinalValues; }


	protected:
		void Construct();

		::connectivity::OSQLParseNode* implPredicateTree(::rtl::OUString& _rErrorMessage, const UniString& _rStatement, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _rxField);

	private:
		DECL_LINK(OnVisitedTimeout, Timer*);
		DECL_LINK(OnValueModified, Control*);
		DECL_LINK(OnEntrySelected, ListBox*);
		DECL_LINK(OnButtonClicked, PushButton*);
		DECL_LINK(OnValueLoseFocus, Control*);
	};

//.........................................................................
}	// namespace dbaui
//.........................................................................

#endif // _DBAUI_PARAMDIALOG_HXX_

