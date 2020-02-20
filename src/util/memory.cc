#include "memory.h"

#include <iostream>

namespace toggl {

RelatedData *ProtectedContainerBase::GetRelatedData() {
    return relatedData_;
}

const RelatedData *ProtectedContainerBase::GetRelatedData() const {
    return relatedData_;
}

}
