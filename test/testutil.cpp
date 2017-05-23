/**
 * @file
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <common/debug_string.h>
#include "testutil.h"

namespace po = boost::program_options;

namespace TestSet
{
    std::string perBuild()   { return "build"; }
    std::string perCommit()  { return "commit"; }
    std::string perNightly() { return "nightly"; }
};

static po::variables_map g_vm;

const po::variables_map &testGetOptions()
{
    return g_vm;
}

void testSetOptions(const po::variables_map &vm)
{
    g_vm = vm;
}

TempDirectory::TempDirectory(const std::string &dir)
    : dir(dir)
{
    boost::filesystem::path path(dir);
    boost::filesystem::remove_all(path);
    boost::filesystem::create_directory(path);
}

TempDirectory::~TempDirectory()
{
    boost::filesystem::path path(dir);
    boost::filesystem::remove_all(path);
}
