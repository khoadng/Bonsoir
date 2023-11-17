#pragma once

#include "bonsoir_windows_plugin.h"

#include <windows.h>

#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>

#include "include/dns_sd.h"
#include "bonsoir_broadcast.h"
#include "bonsoir_discovery.h"
#include "bonsoir_service.h"

#include <flutter/method_codec.h>

using namespace flutter;

namespace bonsoir_windows {
    void BonsoirWindowsPlugin::RegisterWithRegistrar(PluginRegistrarWindows* registrar) {
        auto messenger = registrar->messenger();
        auto channel = std::make_unique<MethodChannel<EncodableValue>>(messenger, "fr.skyost.bonsoir", &StandardMethodCodec::GetInstance());
        auto plugin = std::make_unique<BonsoirWindowsPlugin>(messenger);
        plugin.get()->messenger = messenger;

        channel->SetMethodCallHandler([plugin_pointer = plugin.get()](const auto& call, auto result) {
            plugin_pointer->HandleMethodCall(call, std::move(result));
        });

        registrar->AddPlugin(std::move(plugin));
    }

    BonsoirWindowsPlugin::~BonsoirWindowsPlugin() {}

    void BonsoirWindowsPlugin::HandleMethodCall(
        const MethodCall <EncodableValue>& method_call,
        std::unique_ptr <MethodResult<EncodableValue>> result
    ) {
        const auto& method = method_call.method_name();
        const auto* arguments = std::get_if<EncodableMap>(method_call.arguments());
        const auto id = std::get<int>(arguments->find(EncodableValue("id"))->second);
        if (method.compare("broadcast.initialize") == 0) {
            BonsoirService service = BonsoirService(std::get<std::string>(arguments->find(EncodableValue("service.name"))->second), std::get<std::string>(arguments->find(EncodableValue("service.type"))->second), std::get<int>(arguments->find(EncodableValue("service.port"))->second));
            // TODO: Implement attributes and ip.
            const BonsoirBroadcast broadcast = BonsoirBroadcast(id, std::get<bool>(arguments->find(EncodableValue("printLogs"))->second), service, messenger, [this, id]() {
                broadcasts.erase(id);
            });
            broadcasts.insert({ id, broadcast });
            result->Success(EncodableValue(true));
        }
        else if (method.compare("broadcast.start") == 0) {
            if (broadcasts.count(id) == 0) {
                result->Success(EncodableValue(false));
                return;
            }
            broadcasts.at(id).start();
            result->Success(EncodableValue(true));
        }
        else if (method.compare("broadcast.stop") == 0) {
            if (broadcasts.count(id) == 0) {
                result->Success(EncodableValue(false));
                return;
            }
            broadcasts.at(id).dispose();
            result->Success(EncodableValue(true));
        }
        else if (method.compare("discovery.initialize") == 0) {
            result->Success(EncodableValue(true));
        }
        else if (method.compare("discovery.start") == 0) {
            result->Success(EncodableValue(true));
        }
        else if (method.compare("discovery.resolveService") == 0) {
            result->Success(EncodableValue(true));
        }
        else if (method.compare("discovery.stop") == 0) {
            result->Success(EncodableValue(true));
        }
        else {
            result->NotImplemented();
        }
    }
}