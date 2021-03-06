// ---------------------------------------------------------------
// AmrData.H
// ---------------------------------------------------------------
#ifndef _AMRDATA_H_
#define _AMRDATA_H_

#include <AMReX_AmrvisConstants.H>
#include <AMReX_Array.H>
#include <AMReX_MultiFab.H>
#include <AMReX_VisMF.H>

#include <vector>
#include <fstream>
#include <list>
#include <string>
using std::list;
using std::string;
using std::vector;

namespace amrex {

class Interpolater;

class AmrData {
  
 protected: 
  // List of grids at each level, level 0 being coarsest.
  Array<Array<MultiFab *> > dataGrids;    // [level][component]
  vector<vector<vector<bool> > > dataGridsDefined;  // [level][component][index]
  Array<Array<VisMF *> > visMF;    // [level][whichMultiFab]
  Array<int> compIndexToVisMFMap;  // [nComp]
  Array<int> compIndexToVisMFComponentMap;  // [nComp]
  
  int finestLevel;   // most refined level available
  Array<int> refRatio; // refinement ratio between level L and L+1
  Array< Array<Real> > dxLevel;  // [level][dim];   // grid spacing
  Array< Array< Array<Real> > > gridLocLo, gridLocHi;
  Array<Box>  probDomain;
  Array<Box>  maxDomain;        // max problem domain size
  Array<Real> probSize, probLo, probHi;
  Real time;
  Array<string> plotVars;
  Array<Real> vfEps, afEps;
  int nComp, nGrow;
  int nRegions;         // number of boundary regions
  int boundaryWidth;    // number of zones in the boundary regions
  int coordSys;
  Array< Array< FArrayBox *> > regions; // [lev][nReg]  ptrs to bndry data
  BoxArray fabBoxArray;  // used only for fileType == Amrvis::FAB
  string plotFileVersion;
  
  Amrvis::FileType fileType;
  bool bCartGrid;
  int  vCartGrid;  // ---- the CartGrid version
  bool bTerrain;
  Array<int> levelSteps;
  
 public:
  AmrData();
  ~AmrData();
  
  bool ReadData(const string &filename, Amrvis::FileType filetype);
  bool ReadNonPlotfileData(const string &filename, Amrvis::FileType filetype);
  
  const Array<string> &PlotVarNames() const { return plotVars; }
  
  int FinestLevel() const		{ return finestLevel; }
  const Array<int> &RefRatio() const      { return refRatio; }
  const BoxArray &boxArray(int level) const {
    if(fileType == Amrvis::FAB || (fileType == Amrvis::MULTIFAB && level == 0)) {
      return fabBoxArray;
    } else {
      // use visMF[][0]:  all boxArrays are
      // guaranteed to be the same for each MultiFab
      return visMF[level][0]->boxArray();
    }
  }
  
  const DistributionMapping& DistributionMap (int level) const {
      if(fileType == Amrvis::FAB || (fileType == Amrvis::MULTIFAB && level == 0)) {
	  return dataGrids[0][0]->DistributionMap();
      } else {
	  return dataGrids[level][0]->DistributionMap();      
      }
  }

  // limits of interior region of computational domain at each level
  const Array<Box> &ProbDomain() const { return probDomain; }
  // physical size of computational domain
  const Array<Real> &ProbSize() const    { return probSize; }
  const Array<Real> &ProbLo()   const    { return probLo;   }
  const Array<Real> &ProbHi()   const    { return probHi;   }
  
  // return physical location of cell at index ix, level lev
  // cellLoc   = location of cell center
  // loNodeLoc = location of node (vertex) on lower left hand corner
  // hiNodeLoc = location of node (vertex) on upper right hand corner
  void  CellLoc(int lev,   IntVect ix, Array<Real> &pos) const;
  void  LoNodeLoc(int lev, IntVect ix, Array<Real> &pos) const;
  void  HiNodeLoc(int lev, IntVect ix, Array<Real> &pos) const;

  // find the IntVect given a physical location
  // returns the intvect, the finest level it is contained on,
  // and the intvect at the given finest level
  void IntVectFromLocation(const int finestFillLevel, const Array<Real> &location,
                           IntVect &ivLoc, int &ivLevel, IntVect &ivFinestFillLev);
  
  const Array< Array< Array<Real> > > &GridLocLo() const { return gridLocLo; }
  const Array< Array< Array<Real> > > &GridLocHi() const { return gridLocHi; }
  const Array< Array<Real> > &DxLevel() const { return dxLevel; }
  const Array<int> &LevelSteps() const { return levelSteps; }
  
  int NComp() const		{ return nComp; }
  int BoundaryWidth() const	{ return boundaryWidth; }
  int NGrow() const	        { return nGrow; }
  int CoordSys() const	        { return coordSys; }
  Real Time() const	        { return time; }
  const string &PlotFileVersion() const { return plotFileVersion; }
  
  // fill a databox using conservative interpolation
  void FillVar(FArrayBox *destFab, const Box &destBox,
               int finestFillLevel, const string &varName, int procWithFabs);
  void FillVar(Array<FArrayBox *> &destFabs, const Array<Box> &destBoxes,
               int finestFillLevel, const string &varName, int procWithFabs);
  void FillVar(MultiFab &destMultiFab, int finestFillLevel,
	       const Array<string> &varNames, const Array<int> &destFillComps);
  void FillVar(MultiFab &destMultiFab, int finestFillLevel,
	       const string &varname, int destcomp = 0);
  
  const string &GetFileName() const { return fileName; }
  
  void SetFileType(Amrvis::FileType type);
  Amrvis::FileType GetFileType() const     { return fileType;  }
  bool CartGrid() const            { return bCartGrid; }
  int  CartGridVersion() const     { return vCartGrid; }
  Real VfEps(int level) const      { return vfEps[level]; }
  void SetVfEps(Real *newvfeps, int finestlev);
  bool Terrain()  const            { return bTerrain; }

  void SetBoundaryWidth(int width) { boundaryWidth = width; }
  
  bool CanDerive(const string &name) const;
  bool CanDerive(const Array<string> &names) const;
  int  NumDeriveFunc() const;
  void ListDeriveFunc(std::ostream &os) const;
  int  StateNumber(const string &statename) const;
  
  // return the finest level <= startLevel that fully contains box b
  // b is defined on startLevel
  int FinestContainingLevel(const Box &b, int startLevel) const;
  
  // return the finest level <= startLevel that intersects box b
  // b is defined on startLevel
  int FinestIntersectingLevel(const Box &b, int startLevel) const;
  
  // number of grids at level which intersect b
  int NIntersectingGrids(int level, const Box &b) const;
  MultiFab &GetGrids(int level, int componentIndex);
  MultiFab &GetGrids(int level, int componentIndex, const Box &onBox);
  void FlushGrids(int componentIndex);
  
  // calculate the min and max values of derived on onBox at level
  // return false if onBox did not intersect any grids
  bool MinMax(const Box &onBox, const string &derived, int level,
              Real &dataMin, Real &dataMax);
  
  static void SetVerbose(bool tf)       { verbose = tf; }
  static bool Verbose()                 { return verbose; }
  static void SetSkipPltLines(int spl)  { skipPltLines = spl; }
  static void SetStaticBoundaryWidth(int bw)  { sBoundaryWidth = bw; }
  
 private:
  string fileName;
  static Amrvis::FileType defaultFileType;
  static bool verbose;
  static int  skipPltLines;
  static int  sBoundaryWidth;
  
  // fill on interior by piecewise constant interpolation
  void FillInterior(FArrayBox &dest, int level, const Box &subbox);
  void Interp(FArrayBox &fine, FArrayBox &crse,
              const Box &fine_box, int lrat);
  void PcInterp(FArrayBox &fine, const FArrayBox &crse,
                const Box &subbox, int lrat);
  FArrayBox *ReadGrid(std::istream &is, int numVar);
  bool DefineFab(int level, int componentIndex, int fabIndex);
};

}

#endif
