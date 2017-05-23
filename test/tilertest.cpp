/**
 * @file
 *
 * Main program for driving CppUnit tests.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <cppunit/Test.h>
#include <cppunit/TestCase.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <boost/program_options.hpp>
#include "testutil.h"


namespace po = boost::program_options;

static void listTests(CppUnit::Test *root, std::string path)
{
    if (!path.empty())
        path += '/';
    path += root->getName();

    std::cout << path << '\n';
    for (int i = 0; i < root->getChildTestCount(); i++)
    {
        CppUnit::Test *sub = root->getChildTestAt(i);
        listTests(sub, path);
    }
}

static po::variables_map processOptions(int argc, const char **argv)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help",                                      "Show help");

    po::options_description test("Test options");
    test.add_options()
        ("test", po::value<std::string>()->default_value("build"), "Choose test")
        ("list",                                      "List all tests")
        ("verbose,v",                                 "Show result of each test as it runs");
    desc.add(test);

#if HAVE_OPENCL
    po::options_description cl("OpenCL options");
    CLH::addOptions(cl);
    desc.add(cl);
#endif

    try
    {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
                  .style(po::command_line_style::default_style & ~po::command_line_style::allow_guessing)
                  .options(desc)
                  .run(), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << '\n';
            exit(0);
        }
        return vm;
    }
    catch (po::error &e)
    {
        std::cerr << e.what() << "\n\n" << desc << '\n';
        std::exit(1);
    }
}

int main(int argc, const char **argv)
{
    try
    {
        po::variables_map vm = processOptions(argc, argv);
        testSetOptions(vm);

        CppUnit::TestSuite *rootSuite = new CppUnit::TestSuite("All tests");
        CppUnit::TestSuite *buildSuite = new CppUnit::TestSuite("build");
        CppUnit::TestSuite *commitSuite = new CppUnit::TestSuite("commit");
        CppUnit::TestSuite *nightlySuite = new CppUnit::TestSuite("nightly");

        CppUnit::TestFactoryRegistry::getRegistry().addTestToSuite(rootSuite);
        CppUnit::TestFactoryRegistry::getRegistry(TestSet::perBuild()).addTestToSuite(buildSuite);
        CppUnit::TestFactoryRegistry::getRegistry(TestSet::perCommit()).addTestToSuite(commitSuite);
        CppUnit::TestFactoryRegistry::getRegistry(TestSet::perNightly()).addTestToSuite(nightlySuite);

        // Chain the subsuites, so that the bigger ones run the smaller ones too
        commitSuite->addTest(buildSuite);
        nightlySuite->addTest(commitSuite);
        rootSuite->addTest(nightlySuite);

        if (vm.count("list"))
        {
            listTests(rootSuite, "");
            return 0;
        }
        std::string path = vm["test"].as<std::string>();

        CppUnit::BriefTestProgressListener listener;
        CppUnit::TextTestRunner runner;
        runner.addTest(rootSuite);
        runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
        if (vm.count("verbose"))
            runner.eventManager().addListener(&listener);
        bool success = runner.run(path, false, true, false);
        return success ? 0 : 1;
    }
    catch (std::invalid_argument &e)
    {
        std::cerr << "\nERROR: " << e.what() << "\n";
        return 2;
    }
}
