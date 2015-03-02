/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef NDN_APP_PREFIX_HELPER_H
#define NDN_APP_PREFIX_HELPER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/application-container.h"
#include "ns3/ptr.h"

#include <map>
#include <list>

namespace ns3 {
namespace ndn {

class L3Protocol;


class AppPrefixHelper : public Object{
public:

  static TypeId
  GetTypeId();

  AppPrefixHelper();

  uint32_t
  GetId() const;

  Ptr<L3Protocol>
  GetL3Protocol() const;

  void
  SetMap(TypeId m_tid, std::string);

  std::map<TypeId, std::string>
  GetMap();

private:
  std::map<TypeId, std::string> m_prefixmap;
  static uint32_t m_idCounter;
  uint32_t m_id;
  Ptr<L3Protocol> m_ndn;

protected:
  virtual void
  NotifyNewAggregate();
  ///< @brief Notify when the object is aggregated to another object (e.g.,
                       /// Node)
};

} // namespace ndn
} // namespace ns3

#endif // NDN_APP_PREFIX_HELPER_H
