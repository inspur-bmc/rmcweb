/*
// Copyright (c) 2018 Intel Corporation
// Copyright (c) 2018 Ampere Computing LLC
/
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

namespace redfish
{

class Power : public Node
{
  public:
    Power(CrowApp& app) :
        Node((app), "/redfish/v1/Chassis/<str>/Power/", std::string())
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
    void getPowerControlInfo(const std::string& chassis_name,
                             std::shared_ptr<AsyncResp> asyncResp)
    {
        nlohmann::json& item = asyncResp->res.jsonValue["PowerControl"][0];
        item["@odata.id"] =
            "/redfish/v1/Chassis/" + chassis_name + "/Power#/PowerControl/0";
        item["MemberId"] = 0;
        item["Name"] = "System Power Control";
        item["PowerConsumedWatts"] = 8000;
        item["PowerRequestedWatts"] = 8500;
        item["PowerAvailableWatts"] = 8500;
        item["PowerCapacityWatts"] = 10000;
        item["PowerAllocatedWatts"] = 8500;
        item["PowerMetrics"]["IntervalInMin"] = {};
        item["PowerMetrics"]["MinConsumedWatts"] = {};
        item["PowerMetrics"]["MaxConsumedWatts"] = {};
        item["PowerMetrics"]["AverageConsumedWatts"] = {};
        item["PowerLimit"]["LimitInWatts"] = {};
        item["PowerLimit"]["LimitException"] = {};
        item["PowerLimit"]["CorrectionInMs"] = {};
        item["RelatedItem"][0] = {{"@odata.id", "/redfish/v1/Chassis/node1"}};
        item["Status"]["State"] = "Enabled";
        item["Status"]["Health"] = "OK";
        item["Status"]["HealthRollup"] = "OK";
    }
    void getVoltagesInfo(const std::string& chassis_name,
                         std::shared_ptr<AsyncResp> asyncResp, int id,
                         const std::string& sensor_name, int sensor_number,
                         int value)
    {
        nlohmann::json& item = asyncResp->res.jsonValue["Voltages"][id];
        item["@odata.id"] = "/redfish/v1/Chassis/" + chassis_name +
                            "/Power#/Voltages/" + std::to_string(id);
        item["MemberId"] = id;
        item["Name"] = sensor_name;
        item["SensorNumber"] = sensor_number;
        item["Status"]["State"] = "Enabled";
        item["Status"]["Health"] = "OK";
        item["ReadingVolts"] = value;
        item["UpperThresholdNonCritical"] = {};
        item["UpperThresholdCritical"] = {};
        item["UpperThresholdFatal"] = {};
        item["LowerThresholdNonCritical"] = {};
        item["LowerThresholdCritical"] = {};
        item["LowerThresholdFatal"] = {};
        item["MinReadingRange"] = {};
        item["MaxReadingRange"] = {};
        item["PhysicalContext"] = "VoltageRegulator";
        item["RelatedItem"][0] = {{"@odata.id", "/redfish/v1/Chassis/node1"}};
    }

    void getPowerSuppliesInfo(const std::string& chassis_name,
                              std::shared_ptr<AsyncResp> asyncResp)
    {
        nlohmann::json& item = asyncResp->res.jsonValue["PowerSupplies"][0];
        item["@odata.id"] =
            "/redfish/v1/Chassis/" + chassis_name + "/Power#/PowerSupplies/0";
        item["MemberId"] = 0;
        item["Name"] = "Power Supply";
        item["Status"]["State"] = "Enabled";
        item["Status"]["Health"] = "OK";
        item["PowerSupplyType"] = "DC";
        item["LineInputVoltageType"] = "DCNeg48V";
        item["LineInputVoltage"] = 48;
        item["PowerCapacityWatts"] = 400;
        item["LastPowerOutputWatts"] = 192;
        item["Model"] = "499253-B21";
        item["Manufacturer"] = "Delta";
        item["FirmwareVersion"] = "2.75";
        item["SerialNumber"] = "1Z00103";
        item["PartNumber"] = "1Z00103a2";
        item["SparePartNumber"] = {};
        item["InputRanges"] = nlohmann::json::array();
        item["IndicatorLED"] = "Off";

        item["RelatedItem"][0] = {{"@odata.id", "/redfish/v1/Chassis/node1"}};
    }

    void getPowerInfo(const std::string& chassis_name,
                      std::shared_ptr<AsyncResp> asyncResp)
    {
        getPowerControlInfo(chassis_name, asyncResp);
        getPowerSuppliesInfo(chassis_name, asyncResp);
        getVoltagesInfo(chassis_name, asyncResp, 0, "VRM1", 11, 12);
        getVoltagesInfo(chassis_name, asyncResp, 1, "P5V", 12, 5);
        getVoltagesInfo(chassis_name, asyncResp, 2, "P3.3V", 13, 3.3);
        getVoltagesInfo(chassis_name, asyncResp, 3, "P1.5V", 14, 1.5);
        getVoltagesInfo(chassis_name, asyncResp, 4, "P3.3V_STBY", 15, 3.3);
    }

    void doGet(crow::Response& res, const crow::Request& req,
               const std::vector<std::string>& params) override
    {
        if (params.size() != 1)
        {
            res.result(boost::beast::http::status::internal_server_error);
            res.end();
            return;
        }
        const std::string& chassis_name = params[0];

        res.jsonValue["@odata.id"] =
            "/redfish/v1/Chassis/" + chassis_name + "/Power";
        res.jsonValue["@odata.type"] = "#Power.v1_2_1.Power";
        res.jsonValue["@odata.context"] = "/redfish/v1/$metadata#Power.Power";
        res.jsonValue["Id"] = "Power";
        res.jsonValue["Name"] = "Power";
        auto asyncResp = std::make_shared<AsyncResp>(res);

        getPowerInfo(chassis_name, asyncResp);
    }
};

} // namespace redfish
