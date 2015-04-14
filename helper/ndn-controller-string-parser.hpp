/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the Clarkson University.
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

#ifndef NDN_CONTROLLER_STRING_H
#define NDN_CONTROLLER_STRING_H

#include <map>
#include <list>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/concept/assert.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <boost/ref.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/algorithm/string.hpp>

namespace ll = boost::lambda;

static const std::string SOURCE_NODE_NAME = "Source_Node_Name:";
static const std::string LINK_INFORMATION = "Link_Information:{";
static const std::string NODE_PREFIX = "Node_Prefix:{";
static const std::string APP_PREFIX = "App_Prefix:{";
static const std::string CALPATH_PREFIX = "Calculated_Path:{";


using namespace std;

namespace ns3 {
namespace ndn {

class NdnControllerString{
public:

  NdnControllerString(string strInitial);

  std::string
  GetSubString(int start_pos, int end_pos);

  std::string
  GetString();

  std::string
  stringAppend(string strApString);

  std::string
  GetSourceNode();

  std::vector<std::string>
  GetLinkInfo();

  std::string
  GetAppPrefixInfo();

  std::string
  extractInformation(string key, string strPattern);

  vector<std::string>
  GetNodePrefixInfo();

  std::string
  SetSourceNode(string strSourceNode);

  std::string
  SetLinkInfo(string strLinkInfo);

  std::string
  SetAppPrefixInfo(string strAppPrefix);

  std::string
  SetNodePrefixInfo(string strNodePrefix);

  std::vector<std::string>
  extractLinkInformation(string key, string strPattern);

  std::string
  SetCalculatedPath(string strPath);

private:
  std::string m_string;
  static uint32_t m_linkinfoCounter;
  static uint32_t m_AppPrefixCounter;
  static uint32_t m_NodePrefixCounter;
};

} // namespace ndn
} // namespace ns3

#endif // NDN_CONTROLLER_STRING_H
