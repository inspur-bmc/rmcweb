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

#include <boost/algorithm/string/replace.hpp>
#include <dbus_utility.hpp>

namespace redfish
{

/**
 * ManagerActionsReset class supports handle POST method for Reset action.
 * The class retrieves and sends data directly to dbus.
 */
class ManagerActionsReset : public Node
{
  public:
    ManagerActionsReset(CrowApp& app) :
        Node(app, "/redfish/v1/Managers/rmc/Actions/Manager.Reset/")
    {
        entityPrivileges = {
            {boost::beast::http::verb::get, {{"Login"}}},
            {boost::beast::http::verb::head, {{"Login"}}},
            {boost::beast::http::verb::patch, {{"ConfigureManager"}}},
            {boost::beast::http::verb::put, {{"ConfigureManager"}}},
            {boost::beast::http::verb::delete_, {{"ConfigureManager"}}},
            {boost::beast::http::verb::post, {{"ConfigureManager"}}}};
    }

  private:
    /**
     * Function handles POST method request.
     * Analyzes POST body message before sends Reset request data to dbus.
     * OpenBMC allows for ResetType is GracefulRestart only.
     */
    void doPost(crow::Response& res, const crow::Request& req,
                const std::vector<std::string>& params) override
    {
        std::string resetType;

        if (!json_util::readJson(req, res, "ResetType", resetType))
        {
            return;
        }

        if (resetType != "GracefulRestart")
        {
            res.result(boost::beast::http::status::bad_request);
            messages::actionParameterNotSupported(res, resetType, "ResetType");
            BMCWEB_LOG_ERROR << "Request incorrect action parameter: "
                             << resetType;
            res.end();
            return;
        }
        doBMCGracefulRestart(res, req, params);
    }

    /**
     * Function transceives data with dbus directly.
     * All BMC state properties will be retrieved before sending reset request.
     */
    void doBMCGracefulRestart(crow::Response& res, const crow::Request& req,
                              const std::vector<std::string>& params)
    {
    }
};

class Manager : public Node
{
  public:
    Manager(CrowApp& app) : Node(app, "/redfish/v1/Managers/rmc/")
    {
        uuid = app.template getMiddleware<crow::persistent_data::Middleware>()
                   .systemUuid;
        entityPrivileges = {
            {boost::beast::http::verb::get, {{"Login"}}},
            {boost::beast::http::verb::head, {{"Login"}}},
            {boost::beast::http::verb::patch, {{"ConfigureManager"}}},
            {boost::beast::http::verb::put, {{"ConfigureManager"}}},
            {boost::beast::http::verb::delete_, {{"ConfigureManager"}}},
            {boost::beast::http::verb::post, {{"ConfigureManager"}}}};
    }

  private:
    void doGet(crow::Response& res, const crow::Request& req,
               const std::vector<std::string>& params) override
    {
        res.jsonValue["@odata.id"] = "/redfish/v1/Managers/rmc";
        res.jsonValue["@odata.type"] = "#Manager.v1_3_0.Manager";
        res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#Manager.Manager";
        res.jsonValue["Id"] = "rmc";
        res.jsonValue["Name"] = "OpenRmc Manager";
        res.jsonValue["Description"] = "RackScale RMC";
        res.jsonValue["PowerState"] = "On";
        res.jsonValue["ManagerType"] = "RMC Manager";
        res.jsonValue["UUID"] = uuid;
        res.jsonValue["Model"] = "OpenRmc"; // TODO(ed), get model

        res.jsonValue["NetworkProtocol"] = {
            {"@odata.id", "/redfish/v1/Managers/rmc/NetworkProtocol"}};

        res.jsonValue["EthernetInterfaces"] = {
            {"@odata.id", "/redfish/v1/Managers/rmc/EthernetInterfaces"}};
        // default oem data
        nlohmann::json& oem = res.jsonValue["Oem"];
        nlohmann::json& oemOpenrmc = oem["OpenRmc"];
        oem["@odata.type"] = "#OemManager.Oem";
        oem["@odata.id"] = "/redfish/v1/Managers/rmc#/Oem";
        oem["@odata.context"] = "/redfish/v1/$metadata#OemManager.Oem";
        oemOpenrmc["@odata.type"] = "#OemManager.OpenRmc";
        oemOpenrmc["@odata.id"] = "/redfish/v1/Managers/rmc#/Oem/OpenRmc";
        oemOpenrmc["@odata.context"] =
            "/redfish/v1/$metadata#OemManager.OpenRmc";

        // Update Actions object.
        nlohmann::json& manager_reset =
            res.jsonValue["Actions"]["#Manager.Reset"];
        manager_reset["target"] =
            "/redfish/v1/Managers/rmc/Actions/Manager.Reset";
        manager_reset["ResetType@Redfish.AllowableValues"] = {
            "GracefulRestart"};

        res.jsonValue["FirmwareVersion"] = "2.1.71.0";
        res.jsonValue["DateTime"] = getDateTime();

        nlohmann::json& serialConsole = res.jsonValue["SerialConsole"];
        serialConsole["ServiceEnabled"] = true;
        serialConsole["MaxConcurrentSessions"] = 1;
        serialConsole["ConnectTypesSupported"] = {{"SSH"}};

        nlohmann::json& status = res.jsonValue["Status"];
        status["State"] = "Enabled";
        status["Health"] = "OK";
        status["HealthRollup"] = {};

        nlohmann::json& link = res.jsonValue["Links"];
        link["ManagerForServers"] = {};
        nlohmann::json& manageerForChassis = link["ManagerForChassis"];
        manageerForChassis.push_back(
            {{"@odata.id", "/redfish/v1/Chassis/chassis1"}});
        manageerForChassis.push_back(
            {{"@odata.id", "/redfish/v1/Chassis/chassis2"}});
        manageerForChassis.push_back(
            {{"@odata.id", "/redfish/v1/Chassis/chassis3"}});
        link["ManagerInChassis"] = {};

        res.end();
    }

    std::string getDateTime() const
    {
        std::array<char, 128> dateTime;
        std::string redfishDateTime("0000-00-00T00:00:00Z00:00");
        std::time_t time = std::time(nullptr);

        if (std::strftime(dateTime.begin(), dateTime.size(), "%FT%T%z",
                          std::localtime(&time)))
        {
            // insert the colon required by the ISO 8601 standard
            redfishDateTime = std::string(dateTime.data());
            redfishDateTime.insert(redfishDateTime.end() - 2, ':');
        }

        return redfishDateTime;
    }

    std::string uuid;
};

class ManagerCollection : public Node
{
  public:
    ManagerCollection(CrowApp& app) : Node(app, "/redfish/v1/Managers/")
    {
        entityPrivileges = {
            {boost::beast::http::verb::get, {{"Login"}}},
            {boost::beast::http::verb::head, {{"Login"}}},
            {boost::beast::http::verb::patch, {{"ConfigureManager"}}},
            {boost::beast::http::verb::put, {{"ConfigureManager"}}},
            {boost::beast::http::verb::delete_, {{"ConfigureManager"}}},
            {boost::beast::http::verb::post, {{"ConfigureManager"}}}};
    }

  private:
    void doGet(crow::Response& res, const crow::Request& req,
               const std::vector<std::string>& params) override
    {
        // Collections don't include the static data added by SubRoute
        // because it has a duplicate entry for members
        res.jsonValue["@odata.id"] = "/redfish/v1/Managers";
        res.jsonValue["@odata.type"] = "#ManagerCollection.ManagerCollection";
        res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#ManagerCollection.ManagerCollection";
        res.jsonValue["Name"] = "Manager Collection";
        res.jsonValue["Members@odata.count"] = 1;
        res.jsonValue["Members"] = {
            {{"@odata.id", "/redfish/v1/Managers/rmc"}}};
        res.end();
    }
};
} // namespace redfish
