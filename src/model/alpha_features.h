#pragma once
namespace toggl {
    class AlphaFeatures
    {
         public:
            AlphaFeatures();
            void ReadAlphaFeatures(Json::Value root);
            bool IsTimelineUiEnabled();
         private:
            bool isTimelineUiEnabled_;
            Logger logger { "alpha_features" };
    };
}

