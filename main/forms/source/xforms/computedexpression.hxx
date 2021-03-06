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



#ifndef _COMPUTEDEXPRESSION_HXX
#define _COMPUTEDEXPRESSION_HXX


// includes for member variables
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>

// forward declaractions
namespace com { namespace sun { namespace star
{ 
    namespace xml
    {
        namespace dom { class XNode; }
        namespace dom { class XNodeset; }
        namespace xpath { class XXPathAPI; }
        namespace xpath { class XXPathObject; }
    }
    namespace container { class XNameContainer; }
} } }
namespace xforms { class EvaluationContext; }



namespace xforms
{

/** ComputedExpression represents an XPath Expression and caches results.
 *
 * As this class has no virtual methods, it should never be used
 * polymorphically. */
class ComputedExpression
{
    /// the expression string
    rtl::OUString msExpression;

    /// is msExpression empty?
    bool mbIsEmpty;

protected:
    /// is msExpression a simple expression?
    bool mbIsSimple;

    /// the result from the last bind
    com::sun::star::uno::Reference<com::sun::star::xml::xpath::XXPathObject> mxResult;


    /// implementation of isSimpleExpression
    bool _checkExpression( const sal_Char* pExpression ) const;

    /// allow manipulation of the expression before it is evaluated
    const rtl::OUString _getExpressionForEvaluation() const;

    /// obtain a (suitable) XPathAPI implementation
    com::sun::star::uno::Reference<com::sun::star::xml::xpath::XXPathAPI> _getXPathAPI(const xforms::EvaluationContext& aContext);

    /// evaluate the expression relative to the content node.
    bool _evaluate( const xforms::EvaluationContext& rContext,
                    const rtl::OUString& sExpression );


public:
    ComputedExpression();
    ~ComputedExpression();


    /// get the expression string
    rtl::OUString getExpression() const;

    /// set a new expression string
    void setExpression( const rtl::OUString& rExpression );

    /// get the namespaces that are used to interpret the expression string
    com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> getNamespaces() const;

    /// set the namespaces that are used to interpret the expression string
    void setNamespaces( const com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>& );

    /// do we have an actual expression?
    bool isEmptyExpression() const;

    /// heuristically determine whether this expression is 'simple',
    /// i.e. whether its value will change depending on the values
    /// of other nodes
    bool isSimpleExpression() const;


    /// evaluate the expression relative to the content node.
    bool evaluate( const xforms::EvaluationContext& rContext );


    /// does this expression have a value?
    bool hasValue() const;


    /// remove value/evaluate results
    void clear();


    // get the result of this expression as string/bool/...
    // (Results will be based on the last call of evaluate(..). The caller
    // must call evaluate to ensure current results.)
	com::sun::star::uno::Reference<com::sun::star::xml::xpath::XXPathObject> getXPath();
    bool getBool( bool bDefault = false ) const;
    rtl::OUString getString( const rtl::OUString& rDefault = rtl::OUString() ) const;

};

} // namespace xforms

#endif
