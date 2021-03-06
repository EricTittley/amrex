/*
 *       {_       {__       {__{_______              {__      {__
 *      {_ __     {_ {__   {___{__    {__             {__   {__  
 *     {_  {__    {__ {__ { {__{__    {__     {__      {__ {__   
 *    {__   {__   {__  {__  {__{_ {__       {_   {__     {__     
 *   {______ {__  {__   {_  {__{__  {__    {_____ {__  {__ {__   
 *  {__       {__ {__       {__{__    {__  {_         {__   {__  
 * {__         {__{__       {__{__      {__  {____   {__      {__
 *
 */

#ifndef _ConductivityBASEDOMAINBC_H___
#define _ConductivityBASEDOMAINBC_H___

#include "AMReX_LoHiSide.H"
#include "AMReX_RealVect.H"
#include "AMReX_FArrayBox.H"
#include "VolIndex.H"
#include "EBCellFAB.H"
#include "EBFaceFAB.H"
#include "EBFluxFAB.H"
#include "EBLevelGrid.H"
#include "EBLevelGrid.H"
#include "TensorCFInterp.H"
#include "BaseBCFuncEval.H"
#include "NamespaceHeader.H"


///
/**
   if neither function or value is set, homogeneousOnly is true
 */
class ConductivityBaseDomainBC: public BaseDomainBC
{
public:
  ///
  /**
   */
  ConductivityBaseDomainBC()
  {
    m_coefSet  = false;
  }
  
  ///
  void
  setValue(Real a_value)
  {
    m_onlyHomogeneous = false;
    m_isFunction = false;
    m_value = a_value;
    m_func = shared_ptr<BaseBCFuncEval>();
  }
  ///
  void
  setFunction(shared_ptr<BaseBCFuncEval> a_func)
  {
    m_value = 12345.6789;
    m_func = a_func;

    m_onlyHomogeneous = false;
    m_isFunction = true;
  }

  ///
  virtual ~ConductivityBaseDomainBC()
  {
  }

  ///
  /**
   */
  void setCoef(EBLevelGrid                      &  a_eblg,
               Real                             &  a_beta,
               Real                             &  a_dx,
               const RealVect                   &  a_probLo,
               shared_ptr<FabArray<EBFluxFAB> > &  a_bcoef)
  {
    m_coefSet = true;
    m_eblg    = a_eblg;
    m_beta    = a_beta;
    m_bcoef   = a_bcoef;
    m_dx      = a_dx;
    m_probLo  = a_probLo;
  }

  Real bcvaluefunc(const RealVect      & a_point,
                   const int           & a_dir,
                   const Side::LoHiSide& a_side)
 {
    Real retval;
    if(m_onlyHomogeneous)
      {
        retval = 0;
      }
    else if(m_isFunction)
      {
        retval = m_func->value(a_point, 0);
      }
    else
      {
        retval = m_value;
      }
    return retval;
  }

  //this makes applyOpRegular faster
  virtual void 
  fillPhiGhost(FArrayBox&     a_state,
               const Box&     a_valid,
               bool           a_homogeneous) = 0;


  //this is for when the EB and the domain intersect
  virtual void getFaceFlux(Real&                 a_faceFlux,
                           const VolIndex&       a_vof,
                           const MFIter    &     a_mfi,
                           const EBCellFAB&      a_phi,
                           const int&            a_idir,
                           const Side::LoHiSide& a_side,
                           const bool&           a_useHomogeneous) = 0;
protected:
  bool m_onlyHomogeneous;
  bool m_isFunction;
  Real m_dx;
  RealVect m_probLo;
  Real m_value;
  bool                              m_coefSet;
  EBLevelGrid                       m_eblg;
  Real                              m_beta;
  shared_ptr<FabArray<EBFluxFAB> >  m_bcoef;
  shared_ptr<BaseBCFuncEval>        m_func;
};

///
/**
 */
class ConductivityBaseDomainBCFactory
{
public:
  ///
  /**
   */
  ConductivityBaseDomainBCFactory()
  {
    m_onlyHomogeneous = true;
  }

  ///
  /**
   */
  virtual ~ConductivityBaseDomainBCFactory()
  {
  }

  ///
  void
  setValue(Real a_value)
  {
    m_onlyHomogeneous = false;
    m_isFunction = false;
    m_value = a_value;
    m_func = shared_ptr<BaseBCFuncEval>();
  }
  ///
  void
  setFunction(shared_ptr<BaseBCFuncEval> a_func)
  {
    m_onlyHomogeneous = false;
    m_value = 12345.6789;
    m_func = a_func;
    m_isFunction = true;

  }
  ///
  /**
   */
  virtual ConductivityBaseDomainBC* new_object_ptr() = 0;

  ///this is the one the operator should actually call
  
  ConductivityBaseDomainBC* create()
    {
      ConductivityBaseDomainBC* newBC = new_object_ptr();
      if (!m_onlyHomogeneous)
      {
        if (m_isFunction)
        {
          newBC->setFunction(m_flux);
        }
        else
        {
          newBC->setValue(m_value);
        }
      }
      else
      {
        newBC->setValue(0.0);
      }
      return static_cast newBC;
    }

protected:
  bool m_onlyHomogeneous;
  bool m_isFunction;
  Real m_value;
  shared_ptr<BaseBCFuncEval>        m_func;

};
}

#endif
