#include "memory.h"

#include <iostream>

namespace toggl {

RelatedData *ProtectedBase::GetRelatedData() {
    return relatedData_;
}

const RelatedData *ProtectedBase::GetRelatedData() const {
    return relatedData_;
}

}
