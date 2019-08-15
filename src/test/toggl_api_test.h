// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TEST_TOGGL_API_TEST_H_
#define SRC_TEST_TOGGL_API_TEST_H_

#if defined(_WIN32) || defined(WIN32)
#define STR(X) L ## X
#else
#define STR(X) X
#endif

#if defined(_WIN32) || defined(WIN32)
#define SRCDIR "../../../../src/"
#else
#define SRCDIR "src/"
#endif

#endif  // SRC_TEST_TOGGL_API_TEST_H_
