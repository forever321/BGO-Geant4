#include "DetectorConstruction.hh"

namespace {

  G4double a;  // atomic mass
  G4double z;  // atomic number
  G4double density,ncomponents,fractionmass,nel;

  G4Element* elH = new G4Element("Hydrogen", "H", z=1., 1.0*g/mole);
  G4Element* elHe = new G4Element("Helium", "He", z=2., 4.00*g/mole);
  G4Element* N = new G4Element("Nitrogen", "N", z=7., a= 14.01*g/mole);
  G4Element* O = new G4Element("Oxygen", "O", z=8., a= 16.00*g/mole);
  G4Element* elMg = new G4Element("Magnesium", "Mg", z=12., a= 24.305*g/mole);
  G4Element* elC = new G4Element("Carbon", "C", z=6., a= 12.01*g/mole);
  G4Material* Pb = new G4Material("Lead", z=82., a= 207.2*g/mole, density= 11.34*g/cm3);
  G4Material* Al = new G4Material("Aluminum", z= 13., a= 26.98*g/mole, density= 2.7*g/cm3);
  G4Material* Si = new G4Material("Silicon", z= 14., a= 28.09*g/mole, density= 2.329*g/cm3);
  G4Element* elGe = new G4Element("Germanium", "Ge", z= 32., a= 72.64*g/mole);
  G4Element* elBi = new G4Element("Bismuth", "Bi", z= 83., a= 208.9*g/mole);
  G4Material* Vacuum = new G4Material("Vacuum", density= 1.37e-7*kg/m3, nel=2);

  double case_diam = 5.78;//cm
  double case_len;//cm

  G4LogicalVolume* case1_log;

}

//-------------------------

DetectorConstruction::DetectorConstruction(DAQManager* aDAQMgr, InputManager* aInMgr) {

  DAQMgr = aDAQMgr;
  InMgr = aInMgr;

}

//-------------------------

DetectorConstruction::~DetectorConstruction() {

}

//-------------------------

G4VPhysicalVolume* DetectorConstruction::Construct() {
//DefineMaterials();
return ConstructDetector();
}

//-------------------------

G4VPhysicalVolume* DetectorConstruction::ConstructDetector() {

  G4SDManager* SDman = G4SDManager::GetSDMpointer();

//------------------------------------------------------ materials

  G4NistManager* man = G4NistManager::Instance();
	
  G4Material* NaI = man->FindOrBuildMaterial("G4_SODIUM_IODIDE");

  G4Material* Air = new G4Material("Air", density= 1.2922*kg/m3, nel=2);
  Air->AddElement(N, 70*perCent);
  Air->AddElement(O, 30*perCent);

  Vacuum->AddElement(N, 70*perCent);
  Vacuum->AddElement(O, 30*perCent);
  
  target = new G4Material("target", density= 10.e-7*g/cm3, nel=1);
  target->AddElement(elH, 100*perCent);
  
  BGO = new G4Material("BGO", density= 7.13*g/cm3, nel=3);
  BGO->AddElement(elBi, 4);//no. el.
  BGO->AddElement(elGe, 3);
  BGO->AddElement(O, 12);

  G4Material* MgO = new G4Material("MgO", density= 3.58*g/cm3, nel=2);
  MgO->AddElement(elMg,1);//no. el.
  MgO->AddElement(O,1);

  G4Material* isobutane = new G4Material("isoC4H10", density= 5.2e-5*g/cm3, nel=2);
  isobutane->AddElement(elC,4);
  isobutane->AddElement(elH,10);

  G4Material* carbonFoil = new G4Material("carbonFoil", density= 2.267*g/cm3, nel=1);
  carbonFoil->AddElement(elC,1);

  G4Material* Mylar = man->FindOrBuildMaterial("G4_MYLAR");
  
//------------------------------------------------------ volumes

//------------------------------ experimental hall (world volume)
//------------------------------ beam line along x axis

  G4Box* expHall_box = new G4Box("expHall_box", 1*m, 1*m, 1*m);
  expHall_log = new G4LogicalVolume(expHall_box, Air, "expHall_log",0,0,0);

  G4VPhysicalVolume* expHall_phys = new G4PVPlacement(0, G4ThreeVector(), expHall_log, "expHall", 0, false, 0);

//------------------------------------------------------------------
//logical volumes:
//------------------------------------------------------------------
//BGO detector case array

  int nz=2; //defines detectors hexagonal geometry
  double* zplane = new double[nz];
  double* rin = new double[nz];
  double* rout = new double[nz]; 

  double crys_diam = 5.58;//cm (diameter of BGO crystal)
  double crys_len = 7.60;//cm (length of BGO crystal)
  double facedepth = 0.3175;//cm (thickness of MgO reflective layer on face of det.)
  double Althickness = 0.0635;//cm (thickness of Al layer surrounding det.)

  case_len = crys_len+facedepth*2.+Althickness;//cm

  zplane[0] = case_len/2.*cm;
  zplane[1] = -case_len/2.*cm;

  rin[0] = 0*cm;
  rin[1] = 0*cm;

  rout[0] = case_diam/2.*cm;
  rout[1] = case_diam/2.*cm;

  G4Polyhedra* case1 = new G4Polyhedra("case1",
                       0, //phi start
                       360*deg, //phi total
                       6, //number of sides
                       2, //number of z planes
                       zplane, //z plane positions
                       rin, //rInner
                       rout //rOuter 
                       ); 
                        	
  case1_log = new G4LogicalVolume(case1,Al,"case1_log");
    
  G4VisAttributes* caseVisAtt = new G4VisAttributes(G4Colour(0.6,0.6,1));
//  caseVisAtt->SetForceSolid(true);
  caseVisAtt->SetForceWireframe(true);
  case1_log->SetVisAttributes(caseVisAtt);  

//------------------------------------------------------------------
//BGO reflector, sides

  double sidedepth = 0.0355;//cm

  zplane[0] = crys_len/2.*cm;
  zplane[1] = -crys_len/2.*cm;

  rin[0] = crys_diam/2.*cm;
  rin[1] = crys_diam/2.*cm;

  rout[0] = (case_diam-2.*Althickness)/2.*cm;
  rout[1] = (case_diam-2.*Althickness)/2.*cm;

  G4Polyhedra* ref1 = new G4Polyhedra("ref1",0,360*deg,6,2,zplane,rin,rout);
  G4LogicalVolume* ref1_log = new G4LogicalVolume(ref1,MgO,"ref1_log");

//------------------------
//front

  zplane[0] = facedepth/2.*cm;
  zplane[1] = -facedepth/2.*cm;

  rin[0] = 0.*cm;
  rin[1] = 0.*cm;

  rout[0] = (case_diam-2.*Althickness)/2.*cm;
  rout[1] = (case_diam-2.*Althickness)/2.*cm;

  G4Polyhedra* ref2 = new G4Polyhedra("ref2",0,360*deg,6,2,zplane,rin,rout);
  G4LogicalVolume* ref2_log = new G4LogicalVolume(ref2,MgO,"ref2_log");

  G4VisAttributes* refVisAtt = new G4VisAttributes(G4Colour(0.,1.,0.));
  refVisAtt->SetForceWireframe(true);
  ref1_log->SetVisAttributes(refVisAtt);
  ref2_log->SetVisAttributes(refVisAtt);  

//------------------------------------------------------------------
//BGO detector crystal

  zplane[0] = crys_len/2.*cm;
  zplane[1] = -crys_len/2.*cm;

  rin[0] = 0*cm;
  rin[1] = 0*cm;

  rout[0] = crys_diam/2.*cm;
  rout[1] = crys_diam/2.*cm;

  G4Polyhedra* crys1 = new G4Polyhedra("crys1",0,360*deg,6,2,zplane,rin,rout); 
                        	
  G4LogicalVolume* crys1_log = new G4LogicalVolume(crys1,BGO,"crys1_log");

  G4VisAttributes* crysVisAtt = new G4VisAttributes(G4Colour(1.,0.,0.));
  crysVisAtt->SetForceWireframe(true);
  crys1_log->SetVisAttributes(crysVisAtt); 

//------------------------------------------------------------

  string GeomType;

  InMgr->GetVariable("GeomType",GeomType);//Regular or single

  if (GeomType == "Regular") ConstructRegular();//geometry type
  if (GeomType == "Single") Single();

  G4String SDname = "SD";
  TrackerSD* aTrackerSD = new TrackerSD("BGO",DAQMgr,InMgr);
  SDman->AddNewDetector(aTrackerSD);
  crys1_log->SetSensitiveDetector(aTrackerSD);

//place physical volumes
  G4VPhysicalVolume* ref1_phys = new G4PVPlacement(0,G4ThreeVector(0,0,0),ref1_log,"MgO",case1_log,false,0);
  G4VPhysicalVolume* ref2_phys = new G4PVPlacement(0,G4ThreeVector(0,0,(crys_len+facedepth)/2.*cm),ref2_log,"MgO",case1_log,false,0);
  G4VPhysicalVolume* ref3_phys = new G4PVPlacement(0,G4ThreeVector(0,0,-(crys_len+facedepth)/2.*cm),ref2_log,"MgO",case1_log,false,0);
  G4VPhysicalVolume* crys1_phys = new G4PVPlacement(0,G4ThreeVector(0,0,0),crys1_log,"BGO",case1_log,false,0);

  return expHall_phys;

}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

void DetectorConstruction::ConstructRegular() {


//------------------------------ central chamber case

//------------------------------ outer

  G4Box* chambercase_box = new G4Box("chambercase_box",8.572*cm,12.859*cm,2.540*cm);
  G4LogicalVolume* chambercase_log = new G4LogicalVolume(chambercase_box,Al,"chambercase_log",0,0,0);
  G4VPhysicalVolume* chambercase_phys = new G4PVPlacement(0,G4ThreeVector(0.,-9.684*cm,0.),chambercase_log,"chambercase",expHall_log,false,0);

  G4VisAttributes* chamberVisAtt = new G4VisAttributes(G4Colour(0.5,0.5,0.5));
  chamberVisAtt->SetForceSolid(true);
//  chamberVisAtt->SetForceWireframe(true);
  chambercase_log->SetVisAttributes(chamberVisAtt);

//------------------------------ inner

  G4Box* chambercase2_box = new G4Box("chambercase2_box",8.255*cm,12.541*cm,2.223*cm);
  G4LogicalVolume* chambercase2_log = new G4LogicalVolume(chambercase2_box,Vacuum,"chambercase2_log",0,0,0);
  G4VPhysicalVolume* chambercase2_phys = new G4PVPlacement(0,G4ThreeVector(0.,0.,0.),chambercase2_log,"chambercase2",chambercase_log,false,0);

  chambercase2_log->SetVisAttributes(chamberVisAtt);

//------------------------------ central chamber

//------------------------------ outer
  
  double theta = 30.*deg;//angle of chamber wall relative to vertical
  double thickness_side = 0.550;//cm
  double thickness_top = 0.317;//cm

  double len_top = 6.759*2.;//cm (full length of outer Al case at top of target)
  double len_bottom = 2.15*2.;//cm (full length of outer Al case at bottom of target)

  G4RotationMatrix* xRot = new G4RotationMatrix;
  xRot->rotateX(90*deg);

  G4Trd* chamber_Trd = new G4Trd("chamber_Trd", len_bottom/2.*cm, len_top/2.*cm, 1.956*cm, 1.956*cm, 4.208*cm);
  G4LogicalVolume* chamber_log = new G4LogicalVolume(chamber_Trd, Al, "chamber_log", 0, 0, 0);
  G4VPhysicalVolume* chamber_phys = new G4PVPlacement(xRot, G4ThreeVector(0.,7.67*cm,0.), chamber_log, "chamber", chambercase2_log, false, 0);

  chamber_log->SetVisAttributes(chamberVisAtt);
   
//------------------------------ inner
   
  G4Trd* chambercav_Trd = new G4Trd("chambercav_Trd", (len_bottom/2.-thickness_side+thickness_top*tan(theta))*cm, (len_top/2.-thickness_side-thickness_top*tan(theta))*cm, 1.905*cm, 1.905*cm, 3.891*cm);
  G4LogicalVolume* chambercav_log = new G4LogicalVolume(chambercav_Trd, target, "chambercav_log", 0, 0, 0);
  G4VPhysicalVolume* chambercav_phys = new G4PVPlacement(0, G4ThreeVector(0.,0.,0.), chambercav_log, "chambercav", chamber_log, false, 0);

  chambercav_log->SetVisAttributes(chamberVisAtt);
/*
//------------------------------ Sb cavity

  G4RotationMatrix* yRot2 = new G4RotationMatrix;
  yRot2->rotateY(61.5*deg);

  G4Box* sbcav_box = new G4Box("sbcav_box",3.810/2.*cm,1.588/2.*cm,0.476/2.*cm);
  G4LogicalVolume* sbcav_log = new G4LogicalVolume(sbcav_box, Vacuum, "sbcav_log");
  G4VPhysicalVolume* sbcav_phys = new G4PVPlacement(yRot2, G4ThreeVector(3.320*cm,0*cm,-1.594*cm), sbcav_log, "sbcav1", chamber_log, false, 0);

  G4VisAttributes* cavVisAtt = new G4VisAttributes(G4Colour(1.,0.,0.));
  cavVisAtt->SetForceSolid(true);

  sbcav_log->SetVisAttributes(cavVisAtt);
*/
//------------------------------ upstream pumping tubes

  G4RotationMatrix* yRot = new G4RotationMatrix;
  yRot->rotateY(90*deg);

  double pumptube_len = 5.08;//cm
  double pumptube_diam = 1.895;//cm (outer)

  G4Tubs* pumpup1_Tubs = new G4Tubs("pumpup1_Tubs",0.59*cm,pumptube_diam/2.*cm,pumptube_len/2.*cm,0*deg,360*deg);
  G4LogicalVolume* pumpup1_log = new G4LogicalVolume(pumpup1_Tubs, Al, "pumpup1_log");
  G4VPhysicalVolume* pumpup1_phys = new G4PVPlacement(yRot, G4ThreeVector(-20.638*cm,0,0), pumpup1_log, "pumpup1", expHall_log, false, 0);

  pumpup1_log->SetVisAttributes(chamberVisAtt);

//

  G4Tubs* pumpup2_Tubs = new G4Tubs("pumpup2_Tubs",0.52*cm,pumptube_diam/2.*cm,pumptube_len/2.*cm,0*deg,360*deg);
  G4LogicalVolume* pumpup2_log = new G4LogicalVolume(pumpup2_Tubs, Al, "pumpup2_log");
  G4VPhysicalVolume* pumpup2_phys = new G4PVPlacement(yRot, G4ThreeVector(-15.558*cm,0,0), pumpup2_log, "pumpup2", expHall_log, false, 0);

  pumpup2_log->SetVisAttributes(chamberVisAtt);

//

  G4Tubs* pumpup3_Tubs = new G4Tubs("pumpup3_Tubs",0.45*cm,pumptube_diam/2.*cm,pumptube_len/2.*cm,0*deg,360*deg);
  G4LogicalVolume* pumpup3_log = new G4LogicalVolume(pumpup3_Tubs, Al, "pumpup3_log");
  G4VPhysicalVolume* pumpup3_phys = new G4PVPlacement(yRot, G4ThreeVector(-10.478*cm,0,0), pumpup3_log, "pumpup3", expHall_log, false, 0);

  pumpup3_log->SetVisAttributes(chamberVisAtt);

//------------------------------ downstream pumping tubes

  G4Tubs* pumpup4_Tubs = new G4Tubs("pumpup4_Tubs",0.50*cm,pumptube_diam/2.*cm,pumptube_len/2.*cm,0*deg,360*deg);
  G4LogicalVolume* pumpup4_log = new G4LogicalVolume(pumpup4_Tubs, Al, "pumpup4_log");
  G4VPhysicalVolume* pumpup4_phys = new G4PVPlacement(yRot, G4ThreeVector(20.638*cm,0,0), pumpup4_log, "pumpup4", expHall_log, false, 0);

  pumpup4_log->SetVisAttributes(chamberVisAtt);

//

  G4Tubs* pumpup5_Tubs = new G4Tubs("pumpup5_Tubs",0.45*cm,pumptube_diam/2.*cm,pumptube_len/2.*cm,0*deg,360*deg);
  G4LogicalVolume* pumpup5_log = new G4LogicalVolume(pumpup5_Tubs, Al, "pumpup5_log");
  G4VPhysicalVolume* pumpup5_phys = new G4PVPlacement(yRot, G4ThreeVector(15.558*cm,0,0), pumpup5_log, "pumpup5", expHall_log, false, 0);

  pumpup5_log->SetVisAttributes(chamberVisAtt);

//

  G4Tubs* pumpup6_Tubs = new G4Tubs("pumpup6_Tubs",0.40*cm,pumptube_diam/2.*cm,pumptube_len/2.*cm,0*deg,360*deg);
  G4LogicalVolume* pumpup6_log = new G4LogicalVolume(pumpup6_Tubs, Al, "pumpup6_log");
  G4VPhysicalVolume* pumpup6_phys = new G4PVPlacement(yRot, G4ThreeVector(10.478*cm,0,0), pumpup6_log, "pumpup6", expHall_log, false, 0);

  pumpup6_log->SetVisAttributes(chamberVisAtt);

//------------------------------ lead shielding (upstream)

  double leadtube_diam = pumptube_diam+2.5;//cm (approx)
  double leadtube_diaminner = pumptube_diam+1.5;//cm (approx)
  double leadtube_len = 6.;//cm

  G4Tubs* lead1_Tubs = new G4Tubs("lead1_Tubs",leadtube_diaminner/2.*cm,leadtube_diam/2.*cm,leadtube_len/2.*cm,0*deg,360*deg);
  G4LogicalVolume* lead1_log = new G4LogicalVolume(lead1_Tubs, Pb, "lead1_log");
  G4VPhysicalVolume* lead1_phys = new G4PVPlacement(yRot, G4ThreeVector(-(8.572+4.2+leadtube_len/2.)*cm,0,0), lead1_log, "lead1", expHall_log, false, 0);

  G4VisAttributes* leadVisAtt = new G4VisAttributes(G4Colour(0.3,0.3,0.3));
  leadVisAtt->SetForceSolid(true);
  lead1_log->SetVisAttributes(leadVisAtt);

//------------------------------ support tube outer (up & downstream)

  double suptubeouter_diam = 3.81;//cm
  double suptubeouter_diaminner = 1.9;//cm
  double suptubeouter_len = 1.27;//cm

  G4Tubs* supout_Tubs = new G4Tubs("supout_Tubs",suptubeouter_diaminner/2.*cm,suptubeouter_diam/2.*cm,suptubeouter_len/2.*cm,0*deg,360*deg);
  G4LogicalVolume* supout_log = new G4LogicalVolume(supout_Tubs, Al, "supout_log");
  G4VPhysicalVolume* supoutdown_phys = new G4PVPlacement(yRot, G4ThreeVector(8.573*cm,0,0), supout_log, "supoutup", expHall_log, false, 0);
  G4VPhysicalVolume* supoutup_phys = new G4PVPlacement(yRot, G4ThreeVector(-8.573*cm,0,0), supout_log, "supoutdown", expHall_log, false, 0);

  supout_log->SetVisAttributes(chamberVisAtt);

//------------------------------ support tube inner (up & downstream)

  double suptubeinner_diam = 2.54;//cm
  double suptubeinner_diaminner = 1.9;//cm
  double suptubeinner_len = 6.985;//cm

  G4Tubs* supin_Tubs = new G4Tubs("supout_Tubs",suptubeinner_diaminner/2.*cm,suptubeinner_diam/2.*cm,suptubeinner_len/2.*cm,0*deg,360*deg);
  G4LogicalVolume* supin_log = new G4LogicalVolume(supin_Tubs, Al, "supin_log");
  G4VPhysicalVolume* supindown_phys = new G4PVPlacement(yRot, G4ThreeVector(12.7*cm,0,0), supin_log, "supinup", expHall_log, false, 0);
  G4VPhysicalVolume* supinup_phys = new G4PVPlacement(yRot, G4ThreeVector(-12.7*cm,0,0), supin_log, "supindown", expHall_log, false, 0);

  supin_log->SetVisAttributes(chamberVisAtt);

//-------------------------------------------------------------------------- BGO array

  G4double xpos;
  G4double ypos;
  G4double zpos;
  G4int copynum;

  G4RotationMatrix* zRot = new G4RotationMatrix;
  zRot->rotateZ(30*deg);

  double array_offset = 0.;//cm 31.5

//----------------------------------------West Array (closest workbench)
//----------------------------------------upper level

  G4VPhysicalVolume* case1_phys;

  xpos = -5.886*cm;
  ypos = 2.686*cm;
  zpos = (2.540+case_len/2.+array_offset)*cm;
  copynum = 15;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 0*cm;
  copynum = 21;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 5.886*cm;
  copynum = 27;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------mid level

  xpos = -8.828*cm;
  ypos = -2.411*cm;
  copynum = 11;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = -2.943*cm;
  copynum = 17;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 2.943*cm;
  copynum = 23;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 8.828*cm;
  copynum = 29;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------lower level

  xpos = -5.885*cm;
  ypos = -7.507*cm;
  copynum = 13;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 0*cm;
  copynum = 19;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 5.885*cm;
  copynum = 25;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);


//----------------------------------------East Array (closest ISAC II walkway)
//----------------------------------------upper level

  xpos = -5.886*cm;
  ypos = 2.686*cm;
  zpos = -(2.540+case_len/2.+array_offset)*cm;
  copynum = 16;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 0*cm;
  copynum = 22;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 5.886*cm;
  copynum = 28;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------mid level

  xpos = -8.828*cm;
  ypos = -2.411*cm;
  copynum = 12;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = -2.943*cm;
  copynum = 18;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 2.943*cm;
  copynum = 24;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 8.828*cm;
  copynum = 30;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------lower level

  xpos = -5.885*cm;
  ypos = -7.507*cm;
  copynum = 14;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 0*cm;
  copynum = 20;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 5.885*cm;
  copynum = 26;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);


//---------------------------------------Crown detectors
//----------------------------------------upper

  xpos = -11.77*cm;
  ypos = 5.376*cm;
  zpos = -(7.3+array_offset)*cm;// 7.3 offset due to lead shielding
  copynum = 3;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = -8.828*cm;
  ypos = 10.473*cm;
  zpos = -array_offset*cm;
  copynum = 4;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 11.77*cm;
  ypos = 5.376*cm;
  zpos = -array_offset*cm;
  copynum = 9;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 8.828*cm;
  ypos = 10.473*cm;
  copynum = 7;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = -2.943*cm;
  ypos = 7.783*cm;
  copynum = 5;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 2.943*cm;
  ypos = 7.783*cm;
  copynum = 6;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------lower

  xpos = -14.713*cm;
  ypos = -4.722*cm;
  zpos = -(7.3+array_offset)*cm;//offset due to lead shielding
  copynum = 1;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = -11.771*cm;
  ypos = -9.819*cm;
  zpos = -array_offset*cm;
  copynum = 2;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 14.713*cm;
  ypos = -4.722*cm;
  copynum = 10;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

//----------------------------------------

  xpos = 11.771*cm;
  ypos = -9.819*cm;
  copynum = 8;
	
  case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,copynum);

}

//------------------------------------------------------------------

void DetectorConstruction::Single() {

//BGO detector casing
    
  G4RotationMatrix* zRot = new G4RotationMatrix;
  zRot->rotateZ(30*deg);

  G4double xpos = 0.*mm;
  G4double ypos = 0.*mm;
  G4double zpos = -(10.+case_len/2.)*cm;
	
  G4VPhysicalVolume* case1_phys = new G4PVPlacement(zRot,G4ThreeVector(xpos,ypos,zpos),case1_log,"case_1",expHall_log,false,0);

}
