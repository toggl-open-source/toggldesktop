#pragma once
namespace toggl {
    class AlphaFeatures
    {
         public:
            AlphaFeatures();
            void ReadAlphaFeatures(const Json::Value &root);
            bool IsSyncEnabled();
            bool IsTimelineUiEnabled();
         private:
            bool isSyncEnabled_;
            bool isTimelineUiEnabled_;
            Logger logger { "alpha_features" };
    };
}

