#ifndef __NUPM_DAX_MAP_H__
#define __NUPM_DAX_MAP_H__

#include <string>
#include "nd_utils.h"

namespace nupm
{

class DM_region_header;

/** 
 * Lowest level persisent manager for devdax devices. See dax_map.cc for static configuration.
 * 
 */
class Devdax_manager
{
private:
  static constexpr unsigned _debug_level = 3;
  
public:
  /** 
   * Constructor
   * 
   * @param force_reset If true, contents will be re-initialized. If false, re-initialization occurs on bad version/magic detection.
   */
  Devdax_manager(bool force_reset = false);
  ~Devdax_manager();

  /** 
   * Open a region of memory
   * 
   * @param uuid Unique identifier
   * @param numa_node NUMA node
   * @param out_length Out length of region in bytes
   * 
   * @return Pointer to mapped memory or nullptr on not found
   */
  void * open_region(uint64_t uuid, int numa_node, size_t * out_length);

  /** 
   * Create a new region of memory
   * 
   * @param uuid Unique identifier
   * @param numa_node NUMA node
   * @param size Size of the region requested in bytes
   * 
   * @return Pointer to mapped memory
   */
  void * create_region(uint64_t uuid, int numa_node, size_t size);

  /** 
   * Erase a previously allocated region
   * 
   * @param uuid Unique region identifier
   * @param numa_node NUMA node
   */
  void erase_region(uint64_t uuid, int numa_node);

  /** 
   * Get the maximum "hole" size.
   * 
   * 
   * @return Size in bytes of max hole
   */
  size_t get_max_available(int numa_node);
  
  /** 
   * Debugging information
   * 
   * @param numa_node 
   */
  void debug_dump(int numa_node);
  
private:
  void * get_devdax_region(const char * device_path, size_t * out_length);
  void * map_region(const char * path, addr_t base_addr);
  void recover_metadata(const char * device_path, void * p, size_t p_len, bool force_rebuild = false);
  
private:
  ND_control                                _nd;
  std::map<std::string, iovec>              _mapped_regions;
  std::map<std::string, DM_region_header *> _region_hdrs;
};


}

#endif
