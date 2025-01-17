// Copyright(C) 1999-2010
// Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
// certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef IOSS_Ioss_IOUtils_h
#define IOSS_Ioss_IOUtils_h

#include <Ioss_CodeTypes.h>
#include <algorithm> // for sort, lower_bound, copy, etc
#include <assert.h>
#include <cstdlib>   // for nullptrr
#include <iostream>  // for ostringstream, etcstream, etc
#include <stddef.h>  // for size_t
#include <stdexcept> // for runtime_error
#include <stdint.h>  // for int64_t
#include <string>    // for string
#include <vector>    // for vector
namespace Ioss {
  class Field;
}
namespace Ioss {
  class GroupingEntity;
  class Region;
  class SideBlock;
  class PropertyManager;
}

#if __cplusplus > 199711L
#define TOPTR(x) x.data()
#else
#define TOPTR(x) (x.empty() ? nullptr : &x[0])
#endif

#define IOSS_ERROR(errmsg) throw std::runtime_error(errmsg.str())
#define IOSS_WARNING std::cerr

namespace Ioss {

  /* \brief Utility methods.
   */
  class Utils
  {
  public:
    Utils()  = default;
    ~Utils() = default;

    // Assignment operator
    // Copy constructor

    static void check_dynamic_cast(const void *ptr)
    {
      if (ptr == nullptr) {
        std::cerr << "INTERNAL ERROR: Invalid dynamic cast returned nullptr\n";
        exit(EXIT_FAILURE);
      }
    }

    template <typename T> static void uniquify(std::vector<T> &vec)
    {
      std::sort(vec.begin(), vec.end());
      vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
      vec.shrink_to_fit();
    }

    template <typename T> static void generate_index(std::vector<T> &index)
    {
      T sum = 0;
      for (size_t i = 0; i < index.size() - 1; i++) {
        T cnt    = index[i];
        index[i] = sum;
        sum += cnt;
      }
      index[index.size() - 1] = sum;
    }

    template <typename T> static T find_index_location(T node, const std::vector<T> &index)
    {
// 0-based node numbering
// index[p] = first node (0-based) on processor p

#if 1
      // Assume data coherence.  I.e., a new search will be close to the
      // previous search.
      static size_t prev = 1;

      size_t nproc = index.size();
      if (prev < nproc && index[prev - 1] <= node && index[prev] > node)
        return prev - 1;

      for (size_t p = 1; p < nproc; p++) {
        if (index[p] > node) {
          prev = p;
          return p - 1;
        }
      }
      assert(1 == 0); // Cannot happen...
      return -1;
#else
      return std::distance(index.begin(), std::upper_bound(index.begin(), index.end(), node)) - 1;
#endif
    }

    // Fill time_string and date_string with current time and date
    // formatted as "HH:MM:SS" for time and "yy/mm/dd" or "yyyy/mm/dd"
    // for date
    static void time_and_date(char *time_string, char *date_string, size_t length);

    static std::string decode_filename(const std::string &filename, int processor,
                                       int num_processors);
    static int decode_entity_name(const std::string &entity_name);
    static std::string encode_entity_name(const std::string &entity_type, int64_t id);

    // Convert 'name' to lowercase and convert spaces to '_'
    static void fixup_name(char *name);
    static void fixup_name(std::string &name);

    // Check whether property 'prop_name' exists and if so, set 'prop_value'
    // based on the property value.  Either "TRUE", "YES", "ON", or 1 for true;
    // or "FALSE", "NO", "OFF", or not equal to 1 for false.
    // Returns true/false depending on whether property found and value set.
    static bool check_set_bool_property(const Ioss::PropertyManager &properties,
					const std::string &prop_name, bool &prop_value);
    
    // Returns true if the property "omitted" exists on "block"
    static bool block_is_omitted(Ioss::GroupingEntity *block);

    // Process the base element type 'base' which has
    // 'nodes_per_element' nodes and a spatial dimension of 'spatial'
    // into a form that the IO system can (hopefully) recognize.
    // Lowercases the name; converts spaces to '_', adds
    // nodes_per_element at end of name (if not already there), and
    // does some other transformations to remove some exodusII ambiguity.
    static std::string fixup_type(const std::string &base, int nodes_per_element, int spatial);

    static std::string uppercase(const std::string &name);
    static std::string lowercase(const std::string &name);

    static int case_strcmp(const std::string &s1, const std::string &s2);

    // Return a string containing information about the current :
    // computing platform. This is used as information data in the
    // created results file to help in tracking when/where/... the file
    // was created.
    static std::string platform_information();

    // The following functions are wrappers around the sierra::Env functions
    // or similar substitutes to reduce coupling of the Sierra framewk to
    // the rest of the IO Subsystem. When compiled without the Framework code,
    // Only these functions need to be reimplemented.
    static void abort();

    // Return a filename relative to the specified working directory (if any)
    // of the current execution. Working_directory must end with '/' or be empty.
    static std::string local_filename(const std::string &relative_filename, const std::string &type,
                                      const std::string &working_directory);

    static int field_warning(const Ioss::GroupingEntity *ge, const Ioss::Field &field,
                             const std::string &inout);

    static void calculate_sideblock_membership(IntVector &face_is_member, const SideBlock *ef_blk,
                                               size_t int_byte_size, const void *element,
                                               const void *sides, int64_t number_sides,
                                               const Region *region);

    // And yet another idiosyncracy of sidesets...
    // The side of an element (especially shells) can be
    // either a face or an edge in the same sideset.  The
    // ordinal of an edge is (local_edge_number+#faces) on the
    // database, but needs to be (local_edge_number) for
    // Sierra...
    //
    // If the sideblock has a "parent_element_topology" and a
    // "topology", then we can determine whether to offset the
    // side ordinals...
    static int64_t get_side_offset(const Ioss::SideBlock *sb);

    static unsigned int hash(const std::string &name);

    // Return a vector of strings containing the lines of the input file.
    // Should only be called by a single processor or each processor will
    // be accessing the file at the same time...
    //
    static void input_file(const std::string &file_name, std::vector<std::string> *lines,
                           size_t max_line_length = 0);

    template <class T> static std::string to_string(const T &t)
    {
      std::ostringstream os;
      os << t;
      return os.str();
    }

    // Many databases have a maximum length for variable names which can
    // cause a problem with variable name length.
    //
    // This routine tries to shorten long variable names to an acceptable
    // length ('max_var_len' characters max).  If the name is already less than this
    // length, it is returned unchanged...
    //
    // Since there is a (good) chance that two shortened names will match,
    // a 2-letter 'hash' code is appended to the end of the variable name.
    //
    // So, we shorten the name to a maximum of 'max_var_len'-3 characters and append a
    // 2 character hash+separator.
    //
    // It also converts name to lowercase and converts spaces to '_'
    static std::string variable_name_kluge(const std::string &name, size_t component_count,
                                           size_t copies, size_t max_var_len);

    // The model for a history file is a single sphere element (1 node, 1 element)
    // This is needed for some applications that read this file that require a "mesh"
    // even though a history file is just a collection of global variables with no
    // real mesh. This routine will add the mesh portion to a history file.
    static void generate_history_mesh(Ioss::Region *region);
  };
}
#endif
