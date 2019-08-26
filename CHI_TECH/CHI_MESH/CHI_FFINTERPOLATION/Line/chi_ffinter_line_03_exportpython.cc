#include "chi_ffinter_line.h"

#include <iostream>
#include <fstream>

#include <chi_mpi.h>
#include <chi_log.h>

extern CHI_MPI chi_mpi;
extern CHI_LOG chi_log;


//###################################################################
/***/
void chi_mesh::FieldFunctionInterpolationLine::
ExportPython(std::string base_name)
{
  std::ofstream ofile;

  std::string fileName = base_name;
  fileName = fileName + std::to_string(chi_mpi.location_id);
  fileName = fileName + std::string(".py");
  ofile.open(fileName);

  ofile
    << "import numpy as np\n"
       "import matplotlib.pyplot as plt\n"
    << "\n";

  std::string offset;
  std::string submod_name;
  if (chi_mpi.location_id == 0)
  {
    submod_name = base_name;
    submod_name = submod_name + std::to_string(chi_mpi.location_id+1);

    if (chi_mpi.process_count>1)
    {
      ofile << "import " << submod_name << "\n\n";
    }

    for (int ff=0; ff<field_functions.size(); ff++)
    {
      ofile
        << "data" << ff<< "=np.zeros([" << interpolation_points.size()
        << ",5])\n";
    }

    offset = std::string("");
  }
  else if (chi_mpi.process_count>1)
  {

    if (chi_mpi.location_id != (chi_mpi.process_count-1))
    {
      submod_name = base_name;
      submod_name = submod_name + std::to_string(chi_mpi.location_id+1);

      ofile << "import " << submod_name << "\n\n";
    }


  }

  for (int ff=0; ff<field_functions.size(); ff++)
  {
    FIELD_FUNCTION_CONTEXT* ff_ctx =
      ff_contexts[ff];

    if (chi_mpi.process_count>1 and chi_mpi.location_id!=0)
    {
      ofile
        << "def AddData" << ff << "(data" << ff << "):\n";

      offset = std::string("  ");
    }
    for (int p=0; p<interpolation_points.size(); p++)
    {
      if ((ff_ctx->interpolation_points_ass_cell[p]<0)  &&
          (chi_mpi.location_id != 0))
      {
        continue;
      }

      ofile << offset << "data" << ff << "[" << p << ",0] = "
            << interpolation_points[p].x << "\n";
      ofile << offset << "data" << ff << "[" << p << ",1] = "
            << interpolation_points[p].y << "\n";
      ofile << offset << "data" << ff << "[" << p << ",2] = "
            << interpolation_points[p].z << "\n";

      double d = delta_d*p;

      ofile << offset << "data" << ff << "[" << p << ",3] = "
            << d << "\n";
      ofile << offset << "data" << ff << "[" << p << ",4] = "
            << ff_ctx->interpolation_points_values[p] << "\n";


    }
    ofile << offset << "done=True\n";
    ofile << "\n\n";
    if ((chi_mpi.process_count>1) &&
        (chi_mpi.location_id != (chi_mpi.process_count-1)))
    {
      ofile << offset << submod_name
      << ".AddData" << ff << "(data" << ff << ")\n";
    }


  }

  if (chi_mpi.location_id == 0)
  {
    ofile << "plt.figure(1)\n";
    for (int ff=0; ff<field_functions.size(); ff++)
    {
      ofile << "plt.plot(data" << ff << "[:,3],data"
      << ff << "[:,4]"
      << ",label=\"" << field_functions[ff]->text_name << "\""
      << ")\n";
    }
    ofile << "plt.legend()\n"
             "plt.grid(which='major')\n";
    ofile << "plt.show()\n";
  }


  ofile.close();

  chi_log.Log(LOG_0)
    << "Exported Python files for field func \""
    << field_functions[0]->text_name
    << "\" to base name \""
    << base_name << "\" Successfully";


}