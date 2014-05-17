/*
 *  Copyright 2013 CNRS
 *
 *  Nirmal Giftsun
 */



#include "sot-collision/sotcollision/sot-collision.hh"

#include "fcl/traversal/traversal_node_bvhs.h"
#include "fcl/traversal/traversal_node_setup.h"
#include "fcl/collision_node.h"
#include "fcl/collision.h"
#include "fcl/BV/BV.h"
#include "fcl/shape/geometric_shapes.h"
#include "fcl/narrowphase/narrowphase.h"
#include "fcl/math/transform.h"
#include "fcl/collision_data.h"
#include "fcl/collision_object.h"
#include "fcl/BVH/BVH_model.h"
//#include "test_fcl_utility.h"
//#include "fcl_resources/config.h"

#include "commands.hh"
#include "test_fcl_utility.h"


#include <jrl/mal/boost.hh>
#include "jrl/mal/matrixabstractlayer.hh"

#include <dynamic-graph/all-commands.h>


namespace ml = maal::boost;

using namespace std;
using namespace boost::assign;
using namespace boost::tuples;
using namespace fcl;
using namespace ml;
using namespace dynamicgraph::sotcollision;
using namespace dynamicgraph::sot;
using namespace ::dynamicgraph::command;
using namespace dynamicgraph;
using namespace collisiontest;

DYNAMICGRAPH_FACTORY_ENTITY_PLUGIN(SotCollision, "SotCollision");

SotCollision::SotCollision(const std::string& inName) :
  Entity(inName),
fclmodelupdateSINTERN(boost::bind(&SotCollision::updatefclmodels,this,_1,_2),sotNOSIGNAL,"sotCollision("+name+")::intern(dummy)::fclmodelupdateSINTERN" ),
collisionDistance( boost::bind(&SotCollision::computeimdVector,this,_1,_2),
fclmodelupdateSINTERN,
"sotCollision("+name+")::output(vector)::"+std::string("collisionDistance")),
collisionJacobian(boost::bind(&SotCollision::computeimdJacobian,this,_1,_2),
fclmodelupdateSINTERN,
"sotCollision("+name+")::output(matrix)::"+std::string("collisionJacobian"))
{

			signalRegistration(collisionJacobian);
			signalRegistration(collisionDistance);

     collisionDistance.addDependency(fclmodelupdateSINTERN);
     collisionJacobian.addDependency(fclmodelupdateSINTERN);

      dimension = 0;
      num_collisionpairs = 0;
 
     UNIT_ROTATION(0,0) = 1; UNIT_ROTATION(0,1) = 0; UNIT_ROTATION(0,2)=1;
     UNIT_ROTATION(1,0) = 0; UNIT_ROTATION(1,1) = 1; UNIT_ROTATION(1,2)=0;
     UNIT_ROTATION(2,0) = 0; UNIT_ROTATION(2,1) = 0; UNIT_ROTATION(2,2)=1;
     UNIT_ROTATION(0,3) = 0;  UNIT_ROTATION(1,3) =0 ; UNIT_ROTATION(2,3)=0;
 
    // genericSignalRefs.push_back( sig );


     //fclmodelupdateSINTERN.setDependencyType(TimeDependency<int>::BOOL_DEPENDENT);

      using namespace ::dynamicgraph::command;
      std::string docstring;
      docstring = docCommandVoid2("Create a jacobian (world frame) signal only for one joint.",
				  "string (signal name)","string (joint name)");
      addCommand(std::string("DoExperimentCollisionCheck"),
		 new commands::DoExperimentCollisionCheck(*this, docstring));
  // getPendulumMass
  docstring =
    "\n"
    "    Get pendulum mass\n"
    "\n";
  addCommand(std::string("getcheck"),
	     new ::dynamicgraph::command::Getter<SotCollision, bool>
	     (*this, &SotCollision::getcheck, docstring));
  docstring =
    "\n"
    "    Get pendulum mass\n"
    "\n";
  addCommand(std::string("getdistance"),
	     new ::dynamicgraph::command::Getter<SotCollision, double>
	     (*this, &SotCollision::getdistance, docstring));	 
	 docstring =
    "\n"
    "    Get pendulum mass\n"
    "\n";
  addCommand(std::string("gettime"),
	     new ::dynamicgraph::command::Getter<SotCollision, double>
	     (*this, &SotCollision::gettime, docstring));	 	
  


  addCommand(std::string("getfclstate"),
	     new ::dynamicgraph::command::Getter<SotCollision, Matrix>
	     (*this, &SotCollision::getfclstate, docstring));	 	
  
  


////// new important and useful commands

    addCommand(std::string("createcollisionlink"),
		     makeCommandVoid4(*this,&SotCollision::createcollisionlink,docstring));

    addCommand(std::string("createcollisionpair"),
		     makeCommandVoid2(*this,&SotCollision::createcollisionpair,docstring));


/////  new important and useful commands
	 	 
/*

    
    "\n"
    "    Update capsular_links\n"
    "\n";
    addCommand(std::string("update_link_model"),
		     new commands::UpdateLinkModel(*this, docstring));	
docstring =
    "\n"
    "    Get fcl collision model info\n"
    "\n";
  addCommand(std::string("getcollisionmodelinfo"),
	     new ::dynamicgraph::command::Getter<SotCollision, std::vector<Tuple>>
	     (*this, &SotCollision::getcollisionmodelinfo, docstring));    
    */
}
 
		   


SotCollision::~SotCollision()
{
}
void SotCollision::experimentcollision()
{

/*
        Timer timer;
    
        float v1,v2,x,y,z;
	    GJKSolver_indep solver;
        int i = 0;
        //gettimeofday(&start, NULL);
        timer.start();
        
        while(i<1){
                
        v1 = 5; 
        v2 = 10 ;
        x= 23; 
        y = 0;
        z = 0; 

	Capsule capsulea (v1, v2);
	Transform3f capsulea_transform (Vec3f (x, y, z));

        v1 = 5; 
        v2 = 10;
        x = 0; 
        y = 0;
        z = 0; 

	Capsule capsuleb (v1, v2);
    Matrix3f rot(0.0,0,1,0,1,0,-1,0,0) ;
	Transform3f capsuleb_transform (rot,Vec3f (x, y, z));
	
	    FCL_REAL penetration = 0.;
	    FCL_REAL dist = 0.;
        Vec3f l1 , l2;
      
	    check = solver.shapeDistance<Capsule, Capsule>(capsulea, capsulea_transform, capsuleb, capsuleb_transform, &dist, &l1, &l2);

       Vec3f l1 , l2;
       FCL_REAL dist;    
       Box boxa(0.64,0.4,0.11);
       Transform3f boxa_transform (Vec3f(0,0,0));
       Box boxb(0.5,0.8,0.11);
       Transform3f boxb_transform (Vec3f(0.77,-0.82,0.025));
       check = solver.shapeDistance(boxb, boxb_transform, boxa, boxa_transform, &dist, &l1, &l2);

       l1 = boxa_transform.transform(l1);

       l2 = boxb_transform.transform(l2);

       std::cout << l1;
       std::cout << l2; */

	    //check = solver.shapeIntersect(capsulea, capsulea_transform, capsuleb, capsuleb_transform, &contact_point, &penetration, &normal);
	    //distance = dist;
        //std::cout << l1;
        //std::cout << l2; 
        //Transform3f temptransform (Vec3f (1, 1, 1));
        //Vec3f a = temptransform.transform(Vec3f (1, 2, 3));
        //std::cout << "vectranform" << std::endl;
        //std::cout << a[0];
        //std::cout << a[1];
        //std::cout << a[2];

        //if (i = 0)
        //{
          //gettimeofday(&start, NULL);
        //}
        //i = i + 1;
        //}
     
        //gettimeofday(&end, NULL);
        //timer.stop();
        //diff = end.tv_usec - start.tv_usec - 15 ;
       // diff = timer.getElapsedTimeInMicroSec() - 15;

  /*
  using namespace fcl;
 // create bvh for box
  Box b1(20,20,20);
  BVHModel<RSS> b1_rss;
  generateBVHModel(b1_rss, b1, Transform3f());


    //get the data from box transformation
  // create bvh for point cloud
  //get the data from point cloud
  int x = 0;
  int y = 0;
  int z = 0;

  int inr = 1;
  std::vector<Vec3f> pc;
  pc.resize(100);
  for(int i = 0;x <=10;x++)
  {
  for(int j = 0;x <=10;x++)
  {
    Vec3f point(i,y,j);
    pc.push_back(point);
  }
  }

  BV bv;
  BVHModel<BV> m1;
  //m1.bv_splitter.reset(new BVSplitter<BV>(split_method));
  m1.beginModel();
  m1.addSubModel(pc);
  m1.endModel();

 
 // ccd query 
  CollisionRequest request;
  request.gjk_solver_type = GST_INDEP;
  CollisionResult result;
   
        */
}	  



int& SotCollision::updatefclmodels(int& dummy,int time)
{

// update the fcl models from current transformation matrix
    for(int i=0;i<dimension;++i)
    {       
       fclstate.resize(dimension,10);

       Matrix T = body_transformation_input[i]->access(time);

       
       Matrix3f rot(T(0,0),T(0,1),T(0,2),T(1,0),T(1,1),T(1,2),T(2,0),T(2,1),T(2,2));
       Vec3f trans(T(0,3),T(1,3),T(2,3));
       
       transform_links[i]= Transform3f(rot,trans);
       transform_links[i] = transform_links[i] * transform_joint_links[i];
       
       Vec3f tpart = transform_links[i].getTranslation();
       Quaternion3f quat = transform_links[i].getQuatRotation();

       fclstate(i,0) = tpart[0];
       fclstate(i,1) = tpart[1]; 
       fclstate(i,2) = tpart[2];
      
       fclstate(i,3) = quat.getX();  
       fclstate(i,4) = quat.getY();
       fclstate(i,5) = quat.getZ();
       fclstate(i,6) = quat.getW(); 

       if(link_info_map[i].link_type == "capsule")
       {

               fclstate(i,7) = capsule_links[link_info_map[i].index].radius; 
               fclstate(i,8) = capsule_links[link_info_map[i].index].lz;  
               fclstate(i,9) = 0;  
       }
       else if(link_info_map[i].link_type == "box")
       {
               fclstate(i,7) = box_links[link_info_map[i].index].side[0]; 
               fclstate(i,8) = box_links[link_info_map[i].index].side[1];   
               fclstate(i,9) = box_links[link_info_map[i].index].side[2];     
       }
   
    }

// calculate inter model distance 
// ONLY INTER CAPSULE DISTANCE COMPUTED. A GENERALIZATION WILL BE INTRODUCED LATER 

    // define solver
    GJKSolver_libccd solver;
    GJKSolver_indep solver1;
    
    // variables for fcl models
    FCL_REAL dist;
    Vec3f l1 , l2;

    // compute the imdm
    //  This matrix is a vector of dimension * dimension 
    // (distance,closestpointinthecorrespondingrowlink)

    
    
    for(int i=0;i<dimension;++i)
    {

				//Capsule linka (capsule_links[i].radius, capsule_links[i].lz);

				Transform3f linka_transform  = transform_links[i];

				imdm.at(i).at(i) = make_tuple(0,(0,0,0));
       
				for(int j=i+1;j<dimension;++j)
				{                

				    if(link_info_map[i].link_type == "capsule")
				    {

								if(link_info_map[j].link_type == "capsule")
								{
                	  check = solver.shapeDistance<Capsule, Capsule>(capsule_links[link_info_map[i].index],transform_links[i], capsule_links[link_info_map[j].index],transform_links[j], &dist, &l1, &l2);

								}
                else if(link_info_map[j].link_type == "box")
								{
                	  check = solver.shapeDistance(capsule_links[link_info_map[i].index],transform_links[i], box_links[link_info_map[j].index],transform_links[j], &dist, &l1, &l2);
								}

				    }
				    else if(link_info_map[i].link_type == "box")
				    {

								if(link_info_map[j].link_type == "capsule")
								{
                	  check = solver.shapeDistance(box_links[link_info_map[i].index],transform_links[i], capsule_links[link_info_map[j].index],transform_links[j], &dist, &l1, &l2);

								}
                else if(link_info_map[j].link_type == "box")
								{
                	  check = solver1.shapeDistance(box_links[link_info_map[i].index],transform_links[i], box_links[link_info_map[j].index],transform_links[j], &dist, &l1, &l2);


                           l1 = transform_links[i].transform(l1);
                           
                           l2 = transform_links[j].transform(l2);
 
                                              
								}

				    }


		        imdm.at(i).at(j) = make_tuple(dist,make_tuple(l1[0],l1[1],l1[2]));

		        imdm.at(j).at(i) = make_tuple(dist,make_tuple(l2[0],l2[1],l2[2])); 
     
		    }
        
     }
   
//compute inter model jacobian

    return dummy;
}


Matrix& SotCollision::computeimdJacobian(Matrix& res, int time)
{

for (int p=0; p < num_collisionpairs;p++)
{

  std::string body0 = collision_pairs[p][0];
  //std::cout << body0 <<std::endl;
  std::string body1 = collision_pairs[p][1];
  //std::cout << body1 <<std::endl;
  // update fcl models
  fclmodelupdateSINTERN(time);
  
  // init the right value from map
  int i = fcl_body_map[body0];
  int j = fcl_body_map[body1];

  //normalized line segment
  Matrix ln;
  ln.resize(1,3); 


  // position input
  MatrixHomogeneous Mi = body_transformation_input[i]->access(time);
  MatrixHomogeneous Mj = body_transformation_input[j]->access(time);
  Matrix Ji = body_jacobian_input[i]->access(time);
  Matrix Jj = body_jacobian_input[j]->access(time);
  /*
  Matrix Vi(6,6),Vj(6,6);
  for( int m=0;m<3;++i )
    for( int n=0;n<3;++j )
      {
	Vi(m,n)=Mi(m,n);
	Vi(m+3,n+3)=Mi(n,m);
	Vi(m+3,n)=0.;
	Vi(m,n+3)=0.;
      }
  Vi.inverse().multiply(Ji,Ji);
  for( int m=0;m<3;++i )
    for( int n=0;n<3;++j )
      {
	Vj(m,n)=Mj(m,n);
	Vj(m+3,n+3)=Mj(n,m);
	Vj(m+3,n)=0.;
	Vj(m,n+3)=0.;
      }
  Vj.inverse().multiply(Jj,Jj);*/

  int cols = Ji.nbCols();
  // resizing result
  res.resize(num_collisionpairs,cols);
  //std::cout << res <<std::endl;

  // compute collision jacobian distance
     // l1 and l2 vectors
     Vec3f l1(imdm[i][j].get<1>().get<0>(),imdm[i][j].get<1>().get<1>(),imdm[i][j].get<1>().get<2>());
     Vec3f l2(imdm[j][i].get<1>().get<0>(),imdm[j][i].get<1>().get<1>(),imdm[j][i].get<1>().get<2>());
     Vec3f l = l1-l2;
     l.normalize();

     //normalized distance vector
     ln(0,0)= l[0]; ln(0,1)=l[1] ; ln(0,2)=l[2];

     // closest point computation in object a and b

     MatrixHomogeneous aclosestpointtransform, bclosestpointtransform = UNIT_ROTATION;
     aclosestpointtransform(0,3) = l1[0];  aclosestpointtransform(1,3) =l1[1] ; aclosestpointtransform(2,3)=l1[2];
     bclosestpointtransform(0,3) = l2[0];  bclosestpointtransform(1,3) =l2[1] ; bclosestpointtransform(2,3)=l2[2];

     MatrixHomogeneous aclosestpointlocalframe, bclosestpointlocalframe;

     // find closest point transform in object a with respect the local joint frame
     aclosestpointlocalframe = Mi.inverse()*aclosestpointtransform;
     bclosestpointlocalframe = Mj.inverse()*bclosestpointtransform;

     // construct antisymmetric matrices of the closest point for both a and b objects
     Matrix ssmaclosestpoint, ssmbclosestpoint;
     ssmaclosestpoint.resize(3,3);
     ssmbclosestpoint.resize(3,3);

     ssmaclosestpoint(0,0) = 0 ; ssmaclosestpoint(0,1) = -aclosestpointlocalframe(2,3); ssmaclosestpoint(0,2) = aclosestpointlocalframe(1,3) ;
     ssmaclosestpoint(1,0) = aclosestpointlocalframe(2,3) ; ssmaclosestpoint(1,1) = 0; ssmaclosestpoint(1,2) = -aclosestpointlocalframe(0,3) ;
     ssmaclosestpoint(2,0) = -aclosestpointlocalframe(1,3); ssmaclosestpoint(2,1) = aclosestpointlocalframe(0,3); ssmaclosestpoint(2,2) = 0;

     ssmbclosestpoint(0,0) = 0 ; ssmbclosestpoint(0,1) = -bclosestpointlocalframe(2,3); ssmbclosestpoint(0,2) = bclosestpointlocalframe(1,3) ;
     ssmbclosestpoint(1,0) = bclosestpointlocalframe(2,3) ; ssmbclosestpoint(1,1) = 0; ssmbclosestpoint(1,2) = -bclosestpointlocalframe(0,3) ;
     ssmbclosestpoint(2,0) = -bclosestpointlocalframe(1,3); ssmbclosestpoint(2,1) = bclosestpointlocalframe(0,3); ssmbclosestpoint(2,2) = 0;
     

     // linear velocity and angular velocity for both joints 
     Matrix Jiv,Jiw,Jjv,Jjw;

     Ji.extract(0,0,3,cols,Jiv);
     Ji.extract(2,0,3,cols,Jiw);
     if(link_info_map[j].link_location == "internal")
     {
        Jj.extract(0,0,3,cols,Jjv);
        Jj.extract(2,0,3,cols,Jjw);
     }

     // point gradient computation
     Matrix abpointgradient;

     if(link_info_map[j].link_location == "internal")
     {
     abpointgradient  = (Jiv-(ssmaclosestpoint*Jiw)) - (Jjv-(ssmbclosestpoint*Jjw));
     }
     else
     {
      abpointgradient  = (Jiv-(ssmaclosestpoint*Jiw));    
     }
     //compute collision jacobian
     Matrix cj;
     //ln 3*1
     //abpoint 3*3
     cj = ln*abpointgradient;
     for (int d=0;d<cols;d++)
     {
      res(p,d) = cj(0,d); 
     }
     
   } 
   return res;
}


Vector& SotCollision::computeimdVector(Vector& res, int time)
{

  fclmodelupdateSINTERN(time);
  res.resize(num_collisionpairs);
for (int p=0; p < num_collisionpairs;p++)
{
  std::string body0 = collision_pairs[p][0];
  std::string body1 = collision_pairs[p][1];
  int i = fcl_body_map[body0];
  int j = fcl_body_map[body1];

	res(p)= imdm[i][j].get<0>();
}
  //std::cout << "The imdvector "<<i<<"is "<<res;

  return res;
}


dynamicgraph::SignalPtr< MatrixHomogeneous,int >& SotCollision::createPositionSignalIN( const std::string& signame)
{


  dynamicgraph::SignalPtr< MatrixHomogeneous,int> * sig
    = new dynamicgraph::SignalPtr< MatrixHomogeneous,int>
    (NULL,"sotCollision("+name+")::input(matrix)::"+signame);

  genericSignalRefs.push_back( sig );
  signalRegistration( *sig );

 

  return *sig;  
}

dynamicgraph::SignalPtr< Matrix,int >& SotCollision::createJacobiansignalIN( const std::string& signame)
{


  dynamicgraph::SignalPtr< Matrix,int> * sig
    = new dynamicgraph::SignalPtr< Matrix,int>
    (NULL,"sotCollision("+name+")::input(matrix)::"+signame);

  genericSignalRefs.push_back( sig );
  signalRegistration( *sig );


  return *sig;  
}



void SotCollision::createcollisionpair(const std::string& body0,const std::string& body1)
{

    num_collisionpairs = num_collisionpairs+1;
    collision_pairs.resize(num_collisionpairs);
    for(int i = 0; i<num_collisionpairs;i++)
    {  
     collision_pairs[i].resize(2); 
    }
    collision_pairs[num_collisionpairs-1][0] = body0;
    collision_pairs[num_collisionpairs-1][1] = body1;

  // init the right value from map
  int i = fcl_body_map[body0];
  int j = fcl_body_map[body1];

    collisionDistance.addDependency( *body_transformation_input[i] );
    collisionDistance.addDependency( *body_transformation_input[j] );


    collisionJacobian.addDependency( *body_jacobian_input[i] );
    collisionJacobian.addDependency( *body_jacobian_input[j] );

}


void SotCollision::createcollisionlink(const std::string& bodyname, const std::string& bodytype,const std::string& bodynaturelocation,const Vector& bodydescription)
{

// set the dimension for fcl checks
    dimension  = dimension+1;
    // resize the model parameters and the transformations 
    capsule_links.reserve(dimension);
    transform_links.resize(dimension);
    // joint-link transformation matrix to be in sync with rviz as orientation representation of ros is different with sot-dynamics
    transform_joint_links.resize(dimension);
    // resize intermodel distance matrix
		imdm.resize(dimension);
		for(int i=0;i<dimension;++i)
		{
		    imdm[i].resize(dimension);
		}  

    // create input signals
    //    - pose signal from sot dynamic
    // create output signals    
    //    - inter fcl model distance signal for each body 
    //    - jacobian signal for each body

    Matrix3f rotation;

      // create input position/jacbian signals which is supposedly to be plugged to sot dynamic position/jacobian signals
        int i = dimension - 1;
        std::string signame_in = bodyname;
        fcl_body_map.insert(std::pair<std::string,int>(signame_in,i) ); 
        body_transformation_input[i] = &createPositionSignalIN(signame_in); 
        body_jacobian_input[i] = &createJacobiansignalIN(std::string("J")+signame_in); 
        collision_body_jacobian_input[i] = &createJacobiansignalIN(std::string("CJ")+signame_in);
      // temporarily consider only capsules
        // create capsules for the body

        //ShapeBase collision_link;
        if(bodytype == "capsule")
        {
		      Capsule capsule (bodydescription(0),bodydescription(1));
		      capsule_links.push_back(capsule);
          link_info link;
          link.link_type = "capsule";
          link.index = capsule_links.size()-1;
          link.link_location = bodynaturelocation;
          link_info_map.insert(std::pair<int,link_info>(i,link)); 
          
        }
        else if(bodytype == "box")
        {
          Box box(bodydescription(0),bodydescription(1),bodydescription(2));
          box_links.push_back(box);
          link_info link;
          link.link_type = "box";
          link.index = box_links.size()-1;
          link.link_location = bodynaturelocation;
          link_info_map.insert(std::pair<int,link_info>(i,link)); 
        }
        

        //create transforms
        Vec3f translation(bodydescription(3),bodydescription(4),bodydescription(5));
        rotation.setEulerZYX(bodydescription(6),bodydescription(7),bodydescription(8));
        Transform3f joint_link_transform(rotation,translation);


        // push joint link transforms extracted from body description
        transform_joint_links[i] = joint_link_transform;             
        // initialize the fcl body transforms
        //transform_links.push_back(joint_link_transform);

      
      fclmodelupdateSINTERN.addDependency( *body_transformation_input[i] );

      fclmodelupdateSINTERN.addDependency(*body_jacobian_input[i]);

}

/*
void SotCollision::updatelinkmodel(transforms)
{
    Quaternion3f quatrotation;
    
    for(i=0;i<5;i++)
    {
        //get joint tranforms 
        Matrix3f rotation(transforms[i][0](0),transforms[i][0](1),transforms[i][0](2)
                          ,transforms[i][1](0),transforms[i][1](1),transforms[i][1](2)
                          ,transforms[i][2](0),transforms[i][0](1),transforms[i][0](2));
        Vec3f translation(transforms[i][0](3),transforms[i][1](3),transforms[i][2](3));
       
        //update transforms
        transform_links[i].setTransform(rotation,translation);

    }

}


std::tuple SotCollision::getcollisionmodelinfo()
{
    
     std::make_tuple();
     capsule_links
     transform_links
     
     std::tuple a;
     return a;

}*/

