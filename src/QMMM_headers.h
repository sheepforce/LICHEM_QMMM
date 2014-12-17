/*

##############################################################################
#                                                                            #
#              FLUKE: Fields Layered Under Kohn-sham Electrons               #
#                             By: Eric G. Kratz                              #
#                                                                            #
##############################################################################

 Headers and globals for FLUKE. This must be the first file
 imported into main().

*/

//Header Files
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <complex>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/LU>
#include <Eigen/QR>
#include <Eigen/SVD>
#include <Eigen/Eigenvalues>
#include <Eigen/StdList>
#include <Eigen/Eigen>
#include <Eigen/StdVector>
using namespace Eigen;
using namespace std;

//Compile options
const bool Jokes = 1; //Print humorous comments
const bool Debug = 0; //Turn debugging on/off
const bool RCOM = 0; //Remove center of mass
const bool RCOMQM = 0; //Center the QM region
const bool Isotrop = 1; //Force isotropic expansion
const double StepMin = 0.01; //Minimum step size
const double StepMax = 1.0; //Maximum step size
const double Centratio= 5.0; //Scales 'step' for centroids
const int Acc_Check = 5000; //Eq steps before checking accratio
const double OptTol = 1e-6; //Criteria to end the optimization

//Move Probabilities for PIMC
//Note: These probabilities allow for multi-particle moves
double BeadProb = 0.55; //Probability to move a single bead
double CentProb = 0.55; //Probability to move a centroid
double VolProb = 0.10; //Volume change probability

//Physical Constants
const double k = 8.6173324e-5; //Boltzmann constant (eV)
const double hbar = 6.58211928e-16; //Reduced Planck Constant (eV)
const double hbarSI = 1.054571726e-34; //Reduced Planck Constant (SI)
const double kb = 0.69503476; //Boltzmann constant (cm-1)
const double kSI = 1.3806488e-23; //Boltzmann constant (SI)
const double m2Ang = 1.0e10; //Angstroms to meters
const double amu2kg = 1.660538921e-27; //Atomic mass units to kg
const double cs = 2.99792458e8; //Speed of light (m)
const double pi = 4*atan(1); //Pi
const double h = 2*pi*hbar; //Planck Constant (eV)
const double SI2eV = 1/(1.602176565e-19); //Convert SI to eV
const double ToeV = amu2kg*SI2eV/(m2Ang*m2Ang); //Convert to eV units
const double C2eV = m2Ang/(4*pi*SI2eV*8.854187817e-12); //Coulomb to eV
const double Masse = 9.10938291e-31; //Mass of an electron (kg)
const double BohrRad = 0.52917721092; //Bohr radius (Ang)
const double Har2eV = 27.21138386; //Hartrees to eV
const double atm2eV = SI2eV*1.01325e-25; //atmA^3 to eV
const double Na = 6.02214179e23; //Avogadro's number
const double kcal2eV = 4184*SI2eV/Na; //kcal/mol to eV

//Globals
string xyzfilename; //Saves the filename given in the arguments
string confilename; //Saves the filename given in the arguments
string regfilename; //Saves the filename given in the arguments
int Ncpus = 1; //Number of processors for QM calculations
int Nfreeze = 0; //Number of frozen atoms
int Npseudo = 0; //Number of pseudo-atoms
int Nbound = 0; //Number of boundary-atoms
int Natoms = 0; //Total number of atoms
int Nqm = 0; //Number of QM atoms
int Nmm = 0; //Number of MM atoms
double step = StepMin; //PIMC step size
double Lx = 500.0; //Box length
double Ly = 500.0; //Box length
double Lz = 500.0; //Box length

//Flags for simulation options
int GEM = 0; //Flag for frozen density QMMM potential
int AMOEBA = 0; //Flag for polarizable QMMM potential
int CHRG = 1; //Flag for point charge QMMM potential
int PBCon = 0; //Flag for the boundary conditions
int Psi4 = 0; //Wrapper flag
int Gaussian = 0; //Wrapper flag
int Tinker = 0; //Wrapper flag
int Amber = 0; //Wrapper flag
bool QMMM = 0; //Flag for the type of wrapper
bool MMonly = 0; //Flag for the type of wrapper
bool QMonly = 0; //Flag for the type of wrapper
bool OptSim = 0; //Flag for energy minimization
bool PIMCSim = 0; //Flag for Monte Carlo
bool SinglePoint = 0; //Flag for energy calculation
bool GauExternal = 0; //Runs Gaussian with External

//Timers
int StartTime = 0; //Time the calculation starts
int EndTime = 0; //Time the calculation ends
int QMTime = 0; //Sum of QM wrapper times
int MMTime = 0; //Sum of MM wrapper times

//Custom data types
struct Coord
{
  double x; //x position
  double y; //y position
  double z; //z position
};

struct QMMMAtom
{
  double x; //Position, x
  double y; //Position, y
  double z; //Position, z
  double q; //Nuclear charge
  double m; //Mass of atom
  bool QMregion; //QM, MM, pseudo-atom, or boundary-atom
  bool MMregion; //QM, MM, pseudo-atom, or boundary-atom
  bool PAregion; //QM, MM, pseudo-atom, or boundary-atom
  bool BAregion; //QM, MM, pseudo-atom, or boundary-atom
  bool Frozen; //Part of a frozen shell
  string QMTyp; //Real atom type
  string MMTyp; //Force field atom type
  int NumTyp; //Numerical atom type (if used)
  int NumClass; //Numerical atom class (if used)
  int id; //Atom number, starts at zero
  vector<int> Bonds; //Connectivity
  double Ep; //Storage for PI energies
  vector<Coord> P; //Array of PI beads
};

struct QMMMSettings
{
  //Input needed for QM wrappers
  string Func; //DFT functional
  string Basis; //Basis set for QM calculations
  string RAM; //Ram for QM calculations
  string Charge; //QM total charge
  string Spin; //QM total spin
  double Eqm; //QM total energy
  //Input needed for MM wrappers
  double Emm; //MM total energy
  //Input needed for MC functions
  string Ensemble; //NVT or NPT
  double Temp; //Temperature
  double Beta; //Inverse temperature
  double Press; //External pressure
  int Neq; //Number of equilibration run steps
  int Nsteps; //Number of production run steps
  int Nbeads; //Number of time-slices or beads
  double accratio; //Target acceptance ratio
  int Nprint; //Number of steps before printing
  string PrintMode; //Print all beads or just centroids
  //Input needed for optimizations
  int MaxOptSteps; //Maximum iterative optimization steps
};

//Function declarations
void PrintFancyTitle();

double Bohring(double);

double CoordDist2(Coord&,Coord&);

string Typing(int);

int RevTyping(string);

void FindTinkerClasses(vector<QMMMAtom>&);

double TinkerWrapper(string,vector<QMMMAtom>&,QMMMSettings&,int);

double AmberWrapper(string,vector<QMMMAtom>&,QMMMSettings&,int);

double LammpsWrapper(string,vector<QMMMAtom>&,QMMMSettings&,int);

double GaussianWrapper(string,vector<QMMMAtom>&,QMMMSettings&,int);

void ExternalGaussian(int&,char**&);

double PsiWrapper(string,vector<QMMMAtom>&,QMMMSettings&,int);

double SpringEnergy(double,double);

double Get_PI_Espring(vector<QMMMAtom>&,QMMMSettings&);

double Get_PI_Epot(vector<QMMMAtom>&,QMMMSettings&);

bool MCMove(vector<QMMMAtom>&,QMMMSettings&);

void Get_Centroid(QMMMAtom&,QMMMSettings&);

Coord Get_COM(vector<QMMMAtom>&,QMMMSettings&);

void Tink2FLUKE(int&,char**&);

void Remove_COM(vector<QMMMAtom>&,QMMMSettings&);

void Print_traj(vector<QMMMAtom>&,fstream&,QMMMSettings&);

void ReadArgs(int&,char**&,fstream&,fstream&,fstream&,fstream&);

void ReadFlukeInput(fstream&,fstream&,fstream&,
     vector<QMMMAtom>&,QMMMSettings&);

void FlukeErrorChecker(QMMMSettings&);

void FlukePrintSettings(QMMMSettings&);

void GetQuotes(vector<string>&);

//Function definitions
#include "Core_funcs.cpp"
#include "Misc_funcs.cpp"
#include "PathIntegral.cpp"
#include "ReactionPath.cpp"
#include "Tink2FLUKE.cpp"
#include "Multipoles.cpp"

//Wrapper definitions
#include "Lepton_eng.cpp"
#include "Gaussian.cpp"
#include "TINKER.cpp"
#include "LAMMPS.cpp"
#include "Amber.cpp"
#include "PSI4.cpp"
