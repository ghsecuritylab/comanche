/*
   Copyright [2018] [IBM Corporation]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

/*
 * Authors:
 *
 */

#ifndef _EVENT_REGISTRATION_H_
#define _EVENT_REGISTRATION_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <rdma/fabric.h> /* fid_t */
#pragma GCC diagnostic pop

class event_consumer;
class event_producer;
struct fid_ep;
struct fid_pep;

class event_registration
{
  event_producer &_ev;
  ::fid_t _ep;
public:
  explicit event_registration(event_producer &ev, event_consumer &ec, ::fid_ep &ep);
  explicit event_registration(event_producer &ev, event_consumer &ec, ::fid_pep &pep);
  event_registration(const event_registration &) = delete;
  event_registration& operator=(const event_registration &) = delete;
  ~event_registration();
};

#endif
