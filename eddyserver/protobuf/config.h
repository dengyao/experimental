/* protobuf config.h for MSVC.  On other platforms, this is generated
 * automatically by autoheader / autoconf / configure. */

/* the location of <hash_map> */
#define HASH_MAP_H <unordered_map>

/* the namespace of hash_map/hash_set */
// Apparently Microsoft decided to move hash_map *back* to the std namespace
// in MSVC 2010:
//   http://blogs.msdn.com/vcblog/archive/2009/05/25/stl-breaking-changes-in-visual-studio-2010-beta-1.aspx
// TODO(kenton):  Use unordered_map instead, which is available in MSVC 2010.
#define HASH_NAMESPACE std

/* the location of <hash_set> */
#define HASH_SET_H <unordered_set>

/* define if the compiler has hash_map */
#define HAVE_HASH_MAP 1

/* define if the compiler has hash_set */
#define HAVE_HASH_SET 1

/* define if you want to use zlib.  See readme.txt for additional
 * requirements. */
// #define HAVE_ZLIB 1

#define HAVE_PTHREAD 1
#define HASH_MAP_CLASS unordered_map
#define HASH_SET_CLASS unordered_set