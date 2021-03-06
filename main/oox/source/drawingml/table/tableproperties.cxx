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



#include "oox/drawingml/table/tableproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/propertyset.hxx"


using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::table;


namespace oox { namespace drawingml { namespace table {

TableProperties::TableProperties()
: mbRtl( sal_False )
, mbFirstRow( sal_False )
, mbFirstCol( sal_False )
, mbLastRow( sal_False )
, mbLastCol( sal_False )
, mbBandRow( sal_False )
, mbBandCol( sal_False )
{
}
TableProperties::~TableProperties()
{
}

void TableProperties::apply( const TablePropertiesPtr& /* rSourceTableProperties */ )
{
}

void CreateTableRows( uno::Reference< XTableRows > xTableRows, const std::vector< TableRow >& rvTableRows )
{
	if ( rvTableRows.size() > 1 )
		xTableRows->insertByIndex( 0, rvTableRows.size() - 1 );
	std::vector< TableRow >::const_iterator aTableRowIter( rvTableRows.begin() );
	uno::Reference< container::XIndexAccess > xIndexAccess( xTableRows, UNO_QUERY_THROW );
	for ( sal_Int32 n = 0; n < xIndexAccess->getCount(); n++ )
	{
		static const rtl::OUString	sHeight( RTL_CONSTASCII_USTRINGPARAM ( "Height" ) );
		Reference< XPropertySet > xPropSet( xIndexAccess->getByIndex( n ), UNO_QUERY_THROW );
		xPropSet->setPropertyValue( sHeight, Any( static_cast< sal_Int32 >( aTableRowIter->getHeight() / 360 ) ) );
		aTableRowIter++;
	}
}

void CreateTableColumns( Reference< XTableColumns > xTableColumns, const std::vector< sal_Int32 >& rvTableGrid )
{
	if ( rvTableGrid.size() > 1 )
		xTableColumns->insertByIndex( 0, rvTableGrid.size() - 1 );
	std::vector< sal_Int32 >::const_iterator aTableGridIter( rvTableGrid.begin() );
	uno::Reference< container::XIndexAccess > xIndexAccess( xTableColumns, UNO_QUERY_THROW );
	for ( sal_Int32 n = 0; n < xIndexAccess->getCount(); n++ )
	{
		static const rtl::OUString	sWidth( RTL_CONSTASCII_USTRINGPARAM ( "Width" ) );
		Reference< XPropertySet > xPropSet( xIndexAccess->getByIndex( n ), UNO_QUERY_THROW );
		xPropSet->setPropertyValue( sWidth, Any( static_cast< sal_Int32 >( *aTableGridIter++ / 360 ) ) );
	}
}

void MergeCells( const uno::Reference< XTable >& xTable, sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nColSpan, sal_Int32 nRowSpan )
{
   if( xTable.is() ) try
   {
       Reference< XMergeableCellRange > xRange( xTable->createCursorByRange( xTable->getCellRangeByPosition( nCol, nRow,nCol + nColSpan - 1, nRow + nRowSpan - 1 ) ), UNO_QUERY_THROW );
       if( xRange->isMergeable() )
               xRange->merge();
   }
   catch( Exception& )
   {
   }
} 

static TableStyle* pDefaultTableStyle = new TableStyle();

//for pptx just has table style id
static void SetTableStyleProperties(TableStyle* &pTableStyle , const sal_Int32& tblFillClr,const sal_Int32& tblTextClr, const sal_Int32& lineBdrClr)
{	
	//whole table fill style and color
	oox::drawingml::FillPropertiesPtr pWholeTabFillProperties( new oox::drawingml::FillProperties );
	pWholeTabFillProperties->moFillType.set(XML_solidFill);
	pWholeTabFillProperties->maFillColor.setSchemeClr(tblFillClr);  
	pWholeTabFillProperties->maFillColor.addTransformation(XML_tint,20000);
	pTableStyle->getWholeTbl().getFillProperties() = pWholeTabFillProperties;
	//whole table text color
	::oox::drawingml::Color tableTextColor;
	tableTextColor.setSchemeClr(tblTextClr);
	pTableStyle->getWholeTbl().getTextColor() = tableTextColor;
	//whole table line border
	oox::drawingml::LinePropertiesPtr pLeftBorder( new oox::drawingml::LineProperties);
	pLeftBorder->moLineWidth = 12700;
	pLeftBorder->moPresetDash = XML_sng;
	pLeftBorder->maLineFill.moFillType.set(XML_solidFill);
	pLeftBorder->maLineFill.maFillColor.setSchemeClr(lineBdrClr);
	pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_left,pLeftBorder));
	pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_right,pLeftBorder));
	pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_top,pLeftBorder));
	pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_bottom,pLeftBorder));
	pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_insideH,pLeftBorder));
	pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_insideV,pLeftBorder));

	//Band1H style
	oox::drawingml::FillPropertiesPtr pBand1HFillProperties( new oox::drawingml::FillProperties );
	pBand1HFillProperties->moFillType.set(XML_solidFill);
	pBand1HFillProperties->maFillColor.setSchemeClr(tblFillClr);	
	pBand1HFillProperties->maFillColor.addTransformation(XML_tint,40000);
	pTableStyle->getBand1H().getFillProperties() = pBand1HFillProperties;
	
	//Band1V style
	pTableStyle->getBand1V().getFillProperties() = pBand1HFillProperties;

    //tet bold for 1st row/last row/column
	::boost::optional< sal_Bool > textBoldStyle(sal_True);
	pTableStyle->getFirstRow().getTextBoldStyle() = textBoldStyle;
	pTableStyle->getLastRow().getTextBoldStyle() = textBoldStyle;
	pTableStyle->getFirstCol().getTextBoldStyle() = textBoldStyle;
	pTableStyle->getLastCol().getTextBoldStyle() = textBoldStyle;
}

 sal_Bool CreateTableStyle(TableStyle* &pTableStyle , const OUString& styleId)
{	
	sal_Bool createdTblStyle = sal_False;
	if(!styleId.compareToAscii("{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}")){           //Medium Style 2 Accenat 1
		pTableStyle = new TableStyle();
		createdTblStyle = sal_True;
        //first row style
        //fill color and type
		oox::drawingml::FillPropertiesPtr pFstRowFillProperties( new oox::drawingml::FillProperties );
		pFstRowFillProperties->moFillType.set(XML_solidFill);
		pFstRowFillProperties->maFillColor.setSchemeClr(XML_accent1);   
		pTableStyle->getFirstRow().getFillProperties() = pFstRowFillProperties;
		//text color
		::oox::drawingml::Color fstRowTextColor;
		fstRowTextColor.setSchemeClr(XML_lt1);
		pTableStyle->getFirstRow().getTextColor() = fstRowTextColor;
		//bottom line border
		oox::drawingml::LinePropertiesPtr pFstBottomBorder( new oox::drawingml::LineProperties);
		pFstBottomBorder->moLineWidth = 38100;
		pFstBottomBorder->moPresetDash = XML_sng;
		pFstBottomBorder->maLineFill.moFillType.set(XML_solidFill);
		pFstBottomBorder->maLineFill.maFillColor.setSchemeClr(XML_lt1);
		pTableStyle->getFirstRow().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_bottom,pFstBottomBorder));

        //last row style
		pTableStyle->getLastRow().getFillProperties() = pFstRowFillProperties;
		pTableStyle->getLastRow().getTextColor() = fstRowTextColor;
		pTableStyle->getLastRow().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_top,pFstBottomBorder));

		//first column style
		pTableStyle->getFirstRow().getFillProperties() = pFstRowFillProperties;
		pTableStyle->getFirstRow().getTextColor() = fstRowTextColor;

		//last column style
		pTableStyle->getLastCol().getFillProperties() = pFstRowFillProperties;
		pTableStyle->getLastCol().getTextColor() = fstRowTextColor;

		SetTableStyleProperties(pTableStyle, XML_accent1, XML_dk1, XML_lt1);
	}
	else if (!styleId.compareToAscii("{21E4AEA4-8DFA-4A89-87EB-49C32662AFE0}"))         //Medium Style 2 Accent 2
	{
		pTableStyle = new TableStyle();
		createdTblStyle = sal_True;
		oox::drawingml::FillPropertiesPtr pFstRowFillProperties( new oox::drawingml::FillProperties );
		pFstRowFillProperties->moFillType.set(XML_solidFill);
		pFstRowFillProperties->maFillColor.setSchemeClr(XML_accent2);   
		pTableStyle->getFirstRow().getFillProperties() = pFstRowFillProperties;

		::oox::drawingml::Color fstRowTextColor;
		fstRowTextColor.setSchemeClr(XML_lt1);
		pTableStyle->getFirstRow().getTextColor() = fstRowTextColor;
		
		oox::drawingml::LinePropertiesPtr pFstBottomBorder( new oox::drawingml::LineProperties);
		pFstBottomBorder->moLineWidth = 38100;
		pFstBottomBorder->moPresetDash = XML_sng;
		pFstBottomBorder->maLineFill.moFillType.set(XML_solidFill);
		pFstBottomBorder->maLineFill.maFillColor.setSchemeClr(XML_lt1);
		pTableStyle->getFirstRow().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_bottom,pFstBottomBorder));

		pTableStyle->getLastRow().getFillProperties() = pFstRowFillProperties;
		pTableStyle->getLastRow().getTextColor() = fstRowTextColor;
		pTableStyle->getLastRow().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_top,pFstBottomBorder));

		pTableStyle->getFirstCol().getFillProperties() = pFstRowFillProperties;
		pTableStyle->getFirstCol().getTextColor() = fstRowTextColor;
	
		pTableStyle->getLastCol().getFillProperties() = pFstRowFillProperties;
		pTableStyle->getLastCol().getTextColor() = fstRowTextColor;		

		SetTableStyleProperties(pTableStyle, XML_accent2, XML_dk1, XML_lt1);
	}
	else if (!styleId.compareToAscii("{C4B1156A-380E-4F78-BDF5-A606A8083BF9}"))         //Medium Style 4 Accent 4
	{
		pTableStyle = new TableStyle();
		createdTblStyle = sal_True;
		SetTableStyleProperties(pTableStyle, XML_accent4, XML_dk1, XML_accent4);
	}

	return createdTblStyle;
} 
//end

const TableStyle& TableProperties::getUsedTableStyle( const ::oox::core::XmlFilterBase& rFilterBase, sal_Bool &isCreateTabStyle )
{
	::oox::core::XmlFilterBase& rBase( const_cast< ::oox::core::XmlFilterBase& >( rFilterBase ) );

	TableStyle* pTableStyle = NULL;
	if ( mpTableStyle )
		pTableStyle = &*mpTableStyle;
	else if ( rBase.getTableStyles() )
	{
		const std::vector< TableStyle >& rTableStyles( rBase.getTableStyles()->getTableStyles() );
		const rtl::OUString aStyleId( getStyleId().getLength() ? getStyleId() : rBase.getTableStyles()->getDefaultStyleId() );
		std::vector< TableStyle >::const_iterator aIter( rTableStyles.begin() );
		while( aIter != rTableStyles.end() )
		{
			if ( const_cast< TableStyle& >( *aIter ).getStyleId() == aStyleId )
			{
				pTableStyle = &const_cast< TableStyle& >( *aIter );
				break;	// we get the correct style
			}
			aIter++;
		}
		//if the pptx just has table style id, but no table style content, we will create the table style ourselves
		if ( !pTableStyle )
		{
			isCreateTabStyle = CreateTableStyle(pTableStyle , aStyleId);
		}
	}
	if ( !pTableStyle )
		pTableStyle = pDefaultTableStyle;
	
	return *pTableStyle;
}

void TableProperties::pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
	const Reference < XPropertySet >& xPropSet, TextListStylePtr pMasterTextListStyle )
{
	TableStyleListPtr( const_cast< ::oox::core::XmlFilterBase& >( rFilterBase ).getTableStyles() );

	uno::Reference< XColumnRowRange > xColumnRowRange(
 		xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Model") ) ), uno::UNO_QUERY_THROW );

	CreateTableColumns( xColumnRowRange->getColumns(), mvTableGrid );
	CreateTableRows( xColumnRowRange->getRows(), mvTableRows );

	sal_Bool mbOwnTblStyle = sal_False;
	const TableStyle& rTableStyle( getUsedTableStyle( rFilterBase, mbOwnTblStyle ) );
	sal_Int32 nRow = 0;
	std::vector< TableRow >::iterator aTableRowIter( mvTableRows.begin() );
	while( aTableRowIter != mvTableRows.end() )
	{
		sal_Int32 nColumn = 0;
		std::vector< TableCell >::iterator aTableCellIter( aTableRowIter->getTableCells().begin() );
		while( aTableCellIter != aTableRowIter->getTableCells().end() )
		{
			TableCell& rTableCell( *aTableCellIter );
			if ( !rTableCell.getvMerge() && !rTableCell.gethMerge() )
			{
				uno::Reference< XTable > xTable( xColumnRowRange, uno::UNO_QUERY_THROW );
				if ( ( rTableCell.getRowSpan() > 1 ) || ( rTableCell.getGridSpan() > 1 ) )
					MergeCells( xTable, nColumn, nRow, rTableCell.getGridSpan(), rTableCell.getRowSpan() );

				Reference< XCellRange > xCellRange( xTable, UNO_QUERY_THROW );
				rTableCell.pushToXCell( rFilterBase, pMasterTextListStyle, xCellRange->getCellByPosition( nColumn, nRow ), *this, rTableStyle,
					nColumn, aTableRowIter->getTableCells().size(), nRow, mvTableRows.size() );
			}
			nColumn++;
			aTableCellIter++;
		}
		nRow++;
		aTableRowIter++;
	}
	
	if(mbOwnTblStyle == sal_True)
	{
		TableStyle* pTableStyle = (TableStyle*)&rTableStyle;
		if(pTableStyle != NULL)
		{
			delete pTableStyle;
			pTableStyle = NULL;
		}
		mbOwnTblStyle = sal_False;
	}
}

} } }
