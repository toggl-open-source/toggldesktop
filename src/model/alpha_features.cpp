#include <iostream>
#include <https_client.h>
#include <json/json.h>

#include "alpha_features.h"
#include "urls.h"
#include "const.h"
#include "types.h"


namespace toggl {

    AlphaFeatures::AlphaFeatures() {
        isTimelineUiEnabled_ = false;
    }

    void AlphaFeatures::ReadAlphaFeatures(Json::Value root) {
        if (root.isMember("alpha_features")) {
            for (auto i : root["alpha_features"]) {
                if (i.isMember("code")) {
                    if (i["code"] == kTimelineUi && i["enabled"].asBool()) {
                        isTimelineUiEnabled_ = true;
                    }
                }
            }
        }
        else {
            logger.log("Syncer - /me/preferences response didn't contain alpha_features");
        }
    }

    bool AlphaFeatures::IsTimelineUiEnabled() {
        return isTimelineUiEnabled_;
    }
}

