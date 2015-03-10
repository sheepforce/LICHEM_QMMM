/*

##############################################################################
#                                                                            #
#              FLUKE: Fields Layered Under Kohn-sham Electrons               #
#                             By: Eric G. Kratz                              #
#                             and Alice Walker                               #
#                                                                            #
##############################################################################

 Functions for data analysis and trajectory processing.

*/

//Trajectory analysis functions
void Print_traj(vector<QMMMAtom>& parts, fstream& traj, QMMMSettings& QMMMOpts)
{
  int Ntot = QMMMOpts.Nbeads*Natoms;
  traj << Ntot << '\n' << '\n';
  for (int i=0;i<Natoms;i++)
  {
    for (int j=0;j<QMMMOpts.Nbeads;j++)
    {
      traj << parts[i].QMTyp << " ";
      traj << parts[i].P[j].x << " ";
      traj << parts[i].P[j].y << " ";
      traj << parts[i].P[j].z << '\n';
    }
  }
  traj.flush();
  return;
};

void BurstTraj(vector<QMMMAtom>& Struct, string& filename,
     QMMMSettings& QMMMOpts)
{
  //Function to split reaction path and path-integral trajectory frames
  int ct; //Generic counter
  stringstream call;
  fstream burstfile;
  string dummy; //Generic string
  //Open new split trajectory file
  call.str("");
  call << "BurstStruct.xyz";
  ct = 1; //Start counting at the second file
  while (CheckFile(call.str()))
  {
    //Avoids overwriting files
    ct += 1;
    call.str("");
    call << "BurstStruct_";
    call << ct << ".xyz";
  }
  burstfile.open(call.str().c_str(),ios_base::out);
  //Print trajectory
  for (int j=0;j<QMMMOpts.Nbeads;j++)
  {
    burstfile << Natoms; //Number of atoms
    burstfile << '\n' << '\n';
    for (int i=0;i<Natoms;i++)
    {
      burstfile << Struct[i].QMTyp << " ";
      burstfile << Struct[i].P[j].x << " ";
      burstfile << Struct[i].P[j].y << " ";
      burstfile << Struct[i].P[j].z << '\n';
    }
  }
  return;
};
