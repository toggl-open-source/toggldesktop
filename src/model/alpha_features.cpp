#include <iostream>
#include <https_client.h>
#include <json/json.h>

#include "alpha_features.h"
#include "urls.h"
#include "const.h"
#include "types.h"


namespace toggl {

    AlphaFeatures::AlphaFeatures() {
        isSyncEnabled_ = false;
        isTimelineUiEnabled_ = false;
    }

    void AlphaFeatures::ReadAlphaFeatures(Json::Value root) {
        if (root.isMember("alpha_features")) {
            for (auto i : root["alpha_features"]) {
                if (i.isMember("code")) {
                    // there was a typo in the initial set of flags, use both variants to be sure
                    if ((i["code"] == kSyncStrategyLegacy1 || i["code"] == kSyncStrategyLegacy2) && i["enabled"].asBool()) {
                        isSyncEnabled_ = true;
                    }
                    else if (i["code"] == kTimelineUi && i["enabled"].asBool()) {
                        isTimelineUiEnabled_ = true;
                    }
                }
            }
        }
        else {
            logger.log("Syncer - /me/preferences response didn't contain alpha_features");
        }
    }

    bool AlphaFeatures::IsSyncEnabled() {
        return isSyncEnabled_;
    }

    bool AlphaFeatures::IsTimelineUiEnabled() {
        return isTimelineUiEnabled_;
    }
}

