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

#include <boost/container/flat_map.hpp>
#include <node.hpp>
#include <utils/json_utils.hpp>

namespace redfish
{

/**
 * SystemsCollection derived class for delivering ComputerSystems Collection
 * Schema
 */
class SystemsCollection : public Node
{
  public:
    SystemsCollection(CrowApp &app) : Node(app, "/redfish/v1/Systems/")
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
    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        BMCWEB_LOG_DEBUG << "Get list of available boards.";
        std::shared_ptr<AsyncResp> asyncResp = std::make_shared<AsyncResp>(res);
        res.jsonValue["@odata.type"] =
            "#ComputerSystemCollection.ComputerSystemCollection";
        res.jsonValue["@odata.id"] = "/redfish/v1/Systems";
        res.jsonValue["@odata.context"] =
            "/redfish/v1/"
            "$metadata#ComputerSystemCollection.ComputerSystemCollection";
        res.jsonValue["Name"] = "Computer System Collection";

        nlohmann::json &systemArrary = res.jsonValue["Members"];
        systemArrary.push_back({{"@odata.id", "/redfish/v1/Systems/system1"}});
        systemArrary.push_back({{"@odata.id", "/redfish/v1/Systems/system2"}});
        systemArrary.push_back({{"@odata.id", "/redfish/v1/Systems/system3"}});
        res.jsonValue["Memebers@odata.count"] = systemArrary.size();
        res.end();
    }
};

/**
 * SystemActionsReset class supports handle POST method for Reset action.
 * The class retrieves and sends data directly to D-Bus.
 */
class SystemActionsReset : public Node
{
  public:
    SystemActionsReset(CrowApp &app) :
        Node(app, "/redfish/v1/Systems/<str>/Actions/ComputerSystem.Reset/",
             std::string())
    {
        entityPrivileges = {
            {boost::beast::http::verb::post, {{"ConfigureComponents"}}}};
    }

  private:
    /**
     * Function handles POST method request.
     * Analyzes POST body message before sends Reset request data to D-Bus.
     */
    void doPost(crow::Response &res, const crow::Request &req,
                const std::vector<std::string> &params) override
    {
    }
};

/**
 * Systems derived class for delivering Computer Systems Schema.
 */
class Systems : public Node
{
  public:
    /*
     * Default Constructor
     */
    Systems(CrowApp &app) :
        Node(app, "/redfish/v1/Systems/<str>/", std::string())
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
    /**
     * Functions triggers appropriate requests on DBus
     */
    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        // Check if there is required param, truly entering this shall be
        // impossible
        if (params.size() != 1)
        {
            messages::internalError(res);
            res.end();
            return;
        }

        const std::string &name = params[0];

        res.jsonValue["@odata.type"] = "#ComputerSystem.v1_5_1.ComputerSystem";
        res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#ComputerSystem.ComputerSystem";
        res.jsonValue["SystemType"] = "Physical";
        res.jsonValue["Description"] = "Computer System";
        res.jsonValue["Boot"]["BootSourceOverrideEnabled"] =
            "Disabled"; // TODO(Dawid), get real boot data
        res.jsonValue["Boot"]["BootSourceOverrideTarget"] =
            "None"; // TODO(Dawid), get real boot data
        res.jsonValue["Boot"]["BootSourceOverrideMode"] =
            "Legacy"; // TODO(Dawid), get real boot data
        res.jsonValue["Boot"]
                     ["BootSourceOverrideTarget@Redfish.AllowableValues"] = {
            "None",      "Pxe",       "Hdd", "Cd",
            "BiosSetup", "UefiShell", "Usb"}; // TODO(Dawid), get real boot
                                              // data
        res.jsonValue["ProcessorSummary"]["Count"] = 2;
        res.jsonValue["ProcessorSummary"]["Status"]["State"] = "Enabled";
        res.jsonValue["MemorySummary"]["TotalSystemMemoryGiB"] = 96;
        res.jsonValue["MemorySummary"]["Status"]["State"] = "Enabled";
        res.jsonValue["@odata.id"] = "/redfish/v1/Systems/" + name;

        res.jsonValue["Processors"] = {
            {"@odata.id", "/redfish/v1/Systems/" + name + "/Processors"}};
        res.jsonValue["Memory"] = {
            {"@odata.id", "/redfish/v1/Systems/" + name + "/Memory"}};
        // TODO Need to support ForceRestart.
        res.jsonValue["Actions"]["#ComputerSystem.Reset"] = {
            {"target",
             "/redfish/v1/Systems/" + name + "/Actions/ComputerSystem.Reset"},
            {"ResetType@Redfish.AllowableValues",
             {"On", "ForceOff", "GracefulRestart", "GracefulShutdown"}}};

        res.jsonValue["LogServices"] = {
            {"@odata.id", "/redfish/v1/Systems/" + name + "/LogServices"}};

        res.end();
    }

    void doPatch(crow::Response &res, const crow::Request &req,
                 const std::vector<std::string> &params) override
    {
    }
};

class ProcessorCollection : public Node
{
  public:
    ProcessorCollection(CrowApp &app) :
        Node(app, "/redfish/v1/Systems/<str>/Processors", std::string())
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
    void getProcessorList(const std::string &chassis_name,
                          std::shared_ptr<AsyncResp> asyncResp)
    {
        nlohmann::json &chassisArray = asyncResp->res.jsonValue["Members"];
        chassisArray = nlohmann::json::array();
        chassisArray.push_back(
            {{"@odata.id",
              "/redfish/v1/Systems/" + chassis_name + "/Processors/cpu0"}});
        chassisArray.push_back(
            {{"@odata.id",
              "/redfish/v1/Systems/" + chassis_name + "/Processors/cpu1"}});
        asyncResp->res.jsonValue["Members@odata.count"] = chassisArray.size();
    }

    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        if (params.size() != 1)
        {
            messages::internalError(res);
            res.end();
            return;
        }
        const std::string &chassisId = params[0];

        res.jsonValue["@odata.type"] =
            "#ProcessorCollection.ProcessorCollection";
        res.jsonValue["@odata.id"] =
            "/redfish/v1/Systems/" + chassisId + "/Processors";
        res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#ProcessorCollection.ProcessorCollection";
        res.jsonValue["Name"] = "Processors Collection";
        auto asyncResp = std::make_shared<AsyncResp>(res);
        getProcessorList(chassisId, asyncResp);
    }
};

class MemoryCollection : public Node
{
  public:
    MemoryCollection(CrowApp &app) :
        Node(app, "/redfish/v1/Systems/<str>/Memory", std::string())
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
    void getMemoryList(const std::string &chassis_name,
                       std::shared_ptr<AsyncResp> asyncResp)
    {
        nlohmann::json &chassisArray = asyncResp->res.jsonValue["Members"];
        chassisArray = nlohmann::json::array();
        chassisArray.push_back(
            {{"@odata.id",
              "/redfish/v1/Systems/" + chassis_name + "/Memory/mem0"}});
        chassisArray.push_back(
            {{"@odata.id",
              "/redfish/v1/Systems/" + chassis_name + "/Memory/mem1"}});
        chassisArray.push_back(
            {{"@odata.id",
              "/redfish/v1/Systems/" + chassis_name + "/Memory/mem2"}});
        chassisArray.push_back(
            {{"@odata.id",
              "/redfish/v1/Systems/" + chassis_name + "/Memory/mem3"}});
        asyncResp->res.jsonValue["Members@odata.count"] = chassisArray.size();
    }

    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        if (params.size() != 1)
        {
            messages::internalError(res);
            res.end();
            return;
        }
        const std::string &chassisId = params[0];

        res.jsonValue["@odata.type"] = "#MemoryCollection.MemoryCollection";
        res.jsonValue["@odata.id"] =
            "/redfish/v1/Systems/" + chassisId + "/Memory";
        res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#MemoryCollection.MemoryCollection";
        res.jsonValue["Name"] = "Memory Collection";
        auto asyncResp = std::make_shared<AsyncResp>(res);
        getMemoryList(chassisId, asyncResp);
    }
};

class Processor : public Node
{
  public:
    /*
     * Default Constructor
     */
    Processor(CrowApp &app) :
        Node(app, "/redfish/v1/Systems/<str>/Processors/<str>", std::string(),
             std::string())
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
    /**
     * Functions triggers appropriate requests on DBus
     */
    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        // Check if there is required param, truly entering this shall be
        // impossible
        if (params.size() != 2)
        {
            messages::internalError(res);
            res.end();
            return;
        }
        const std::string &systemId = params[0];
        const std::string &processorId = params[1];

        res.jsonValue["@odata.id"] =
            "/redfish/v1/Systems/" + systemId + "/Processors/" + processorId;
        res.jsonValue["@odata.type"] = "#Processor.v1_3_2.Processor";
        res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#Processor.Processor";
        res.jsonValue["Manufacturer"] = "Intel";
        res.jsonValue["Model"] = "Intel(R) Xeon(R) Platinum 8176 CPU @ 2.10GHz";
        res.jsonValue["MaxSpeedMHz"] = 2100;
        res.jsonValue["TDPWatts"] = 165;
        res.jsonValue["TotalCores"] = 28;

        nlohmann::json &status = res.jsonValue["Status"];
        status["State"] = "Enabled";
        status["Health"] = "OK";
        status["HealthRollup"] = {};
        res.end();
    }
};
class Memory : public Node
{
  public:
    /*
     * Default Constructor
     */
    Memory(CrowApp &app) :
        Node(app, "/redfish/v1/Systems/<str>/Memory/<str>", std::string(),
             std::string())
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
    /**
     * Functions triggers appropriate requests on DBus
     */
    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        // Check if there is required param, truly entering this shall be
        // impossible
        if (params.size() != 2)
        {
            messages::internalError(res);
            res.end();
            return;
        }
        // Check if there is required param, truly entering this shall be

        const std::string &systemId = params[0];
        const std::string &memId = params[1];

        res.jsonValue["@odata.id"] =
            "/redfish/v1/Systems/" + systemId + "/Memory/" + memId;
        res.jsonValue["@odata.type"] = "#Memory.v1_0_0.Memory";
        res.jsonValue["@odata.context"] = "/redfish/v1/$metadata#Memory.Memory";
        res.jsonValue["CapacityMiB"] = 16 * 1024;
        res.jsonValue["Meanufacturer"] = "Samsung";
        if (memId == "mem0")
            res.jsonValue["SerialNumber"] = "390FB680";
        else if (memId == "mem1")
            res.jsonValue["SerialNumber"] = "393BE2A3";
        else if (memId == "mem2")
            res.jsonValue["SerialNumber"] = "390FCBBD";
        else if (memId == "mem3")
            res.jsonValue["SerialNumber"] = "39359D7F";
        else
            res.jsonValue["SerialNumber"] = "39359D93";
        nlohmann::json &status = res.jsonValue["Status"];
        status["State"] = "Enabled";
        status["Health"] = "OK";
        status["HealthRollup"] = {};

        res.jsonValue["AllowedSpeedsMHz"] = 2133;
        res.end();
    }
};
} // namespace redfish
