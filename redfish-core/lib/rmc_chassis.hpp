/*
// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
#pragma once

#include "node.hpp"

#include <boost/container/flat_map.hpp>

namespace redfish
{

class ChassisCollection : public Node
{
  public:
    ChassisCollection(CrowApp &app) : Node(app, "/redfish/v1/Chassis/")
    {
        entityPrivileges = {
            {boost::beast::http::verb::get, {{"Login"}}},
            {boost::beast::http::verb::head, {{"Login"}}},
            {boost::beast::http::verb::patch, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::put, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::delete_, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::post, {{"ConfigureComponents"}}}};
    }

  private:
    void getChassisList(std::shared_ptr<AsyncResp> asyncResp)
    {
        nlohmann::json &chassisArray = asyncResp->res.jsonValue["Members"];
        chassisArray = nlohmann::json::array();
        chassisArray.push_back({{"@odata.id", "/redfish/v1/Chassis/chassis1"}});
        chassisArray.push_back({{"@odata.id", "/redfish/v1/Chassis/chassis2"}});
        chassisArray.push_back({{"@odata.id", "/redfish/v1/Chassis/chassis3"}});
        asyncResp->res.jsonValue["Members@odata.count"] = chassisArray.size();
    }

    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        BMCWEB_LOG_ERROR << "doGet in chassis collectoin";
        res.jsonValue["@odata.type"] = "#ChassisCollection.ChassisCollection";
        res.jsonValue["@odata.id"] = "/redfish/v1/Chassis";
        res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#ChassisCollection.ChassisCollection";
        res.jsonValue["Name"] = "Chassis Collection";
        auto asyncResp = std::make_shared<AsyncResp>(res);
        getChassisList(asyncResp);
    }
};

/**
 * Chassis override class for delivering Chassis Schema
 */
class Chassis : public Node
{
  public:
    Chassis(CrowApp &app) :
        Node(app, "/redfish/v1/Chassis/<str>/", std::string())
    {
        entityPrivileges = {
            {boost::beast::http::verb::get, {{"Login"}}},
            {boost::beast::http::verb::head, {{"Login"}}},
            {boost::beast::http::verb::patch, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::put, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::delete_, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::post, {{"ConfigureComponents"}}}};
    }

  private:
    void getChassis(std::shared_ptr<AsyncResp> asyncResp,
                    const std::string &chassisId)
    {

        asyncResp->res.jsonValue["Name"] = chassisId;
        asyncResp->res.jsonValue["Id"] = chassisId;
        asyncResp->res.jsonValue["Thermal"] = {
            {"@odata.id", "/redfish/v1/Chassis/" + chassisId + "/Thermal"}};
        asyncResp->res.jsonValue["Power"] = {
            {"@odata.id", "/redfish/v1/Chassis/" + chassisId + "/Power"}};
    }

    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        // Check if there is required param, truly entering this shall be
        // impossible.
        if (params.size() != 1)
        {
            messages::internalError(res);
            res.end();
            return;
        }

        const std::string &chassisId = params[0];
        std::string ipaddr = "192.168.1.100";
        if (chassisId == "chassis1")
            ipaddr = "192.168.1.101";
        else if (chassisId == "chassis2")
            ipaddr = "192.168.1.102";
        else if (chassisId == "chassis3")
            ipaddr = "192.168.1.103";

        res.jsonValue["@odata.type"] = "#Chassis.v1_4_0.Chassis";
        res.jsonValue["@odata.id"] =
            std::string("/redfish/v1/Chassis/") + chassisId;
        res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#Chassis.Chassis";
        res.jsonValue["Description"] = "Ocp Node";
        res.jsonValue["Manufacturer"] = "Inspur";
        res.jsonValue["Model"] = "OCP";
        res.jsonValue["SKU"] = "2810-9827-5523-1435";
        res.jsonValue["SerialNumber"] = "EE1D253";
        res.jsonValue["PartNumber"] = "NF1750";
        res.jsonValue["IpAddr"] = ipaddr;
        res.jsonValue["AssetTag"] = {};
        res.jsonValue["IndicatorLED"] = {};
        res.jsonValue["PowerState"] = "On";
        res.jsonValue["ChassisType"] = "Rack";
        nlohmann::json &status = res.jsonValue["Status"];
        status["State"] = "Enabled";
        status["Health"] = "OK";
        status["HealthRollup"] = {};

        res.jsonValue["Links"] = {};
        res.jsonValue["ContainedBy"] = {};
        res.jsonValue["ComputerSystems"] = {};
        res.jsonValue["ManagedBy"] = {
            {"@odata.id", "/redfish/v1/Managers/rmc"}};
        res.jsonValue["ManagersInChassis"] = {};
        res.jsonValue["PoweredBy"] = {};
        res.jsonValue["CooledBy"] = {};
        res.jsonValue["Storage"] = {};
        res.jsonValue["Drives"] = {};

        auto asyncResp = std::make_shared<AsyncResp>(res);
        getChassis(asyncResp, chassisId);
    }
};
} // namespace redfish
