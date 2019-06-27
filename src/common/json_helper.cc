#include "json_helper.h"

#include <sstream>

namespace toggl {

    toggl::JsonReader *JsonHelper::reader() {
        static Json::CharReaderBuilder builder;
        static Json::CharReader *internalReader = builder.newCharReader();
        static toggl::JsonReader reader(internalReader);
        return &reader;
    }

    JsonWriter *JsonHelper::writer() {
        static Json::StreamWriterBuilder builder;
        static Json::StreamWriter *internalWriter = builder.newStreamWriter();
        static toggl::JsonWriter writer(internalWriter);
        return &writer;
    }

    JsonReader::JsonReader(Json::CharReader *internal)
        : internal(internal)
    {

    }

    bool JsonReader::parse(std::string document, Json::Value *value, std::string *errors) {
        return internal->parse(document.c_str(), document.c_str() + document.size(), value, errors);
    }

    JsonWriter::JsonWriter(Json::StreamWriter *internal)
        : internal(internal)
    {

    }

    std::string JsonWriter::write(const Json::Value &value) {
        std::stringstream ss;
        internal->write(value, &ss);
        return ss.str();
    }

}
