/**
 * @file
 */

#ifndef UTS_TEST_TESTUTIL_H
#define UTS_TEST_TESTUTIL_H

#include <array>
#include <utility>
#include <cppunit/TestAssert.h>
#include <boost/program_options.hpp>
//#include <common/debug_string.h>

namespace TestSet
{
    std::string perBuild();
    std::string perCommit();
    std::string perNightly();
};

/// Return the command-line options passed to the test executable
const boost::program_options::variables_map &testGetOptions();

/// Set the values returned by @ref testGetOptions
void testSetOptions(const boost::program_options::variables_map &vm);

/**
 * RAII wrapper around creation and destruction of a temporary directory.
 * On construction, the directory is first @em deleted if it exists, then
 * created. On destruction, the directory is deleted.
 *
 * @warning If a relative directory name is used, do not change the current
 * working directory during the lifetime of this object, as otherwise the
 * wrong directory will be removed on destruction.
 */
class TempDirectory
{
private:
    const std::string dir;

    TempDirectory(const TempDirectory &) = delete;
    TempDirectory &operator=(const TempDirectory &) = delete;

public:
    explicit TempDirectory(const std::string &dir);
    ~TempDirectory();
};

namespace detail
{

/// Extended version of @c CppUnit::assertEquals that takes a @a shortDescription parameter
template<typename T>
void assertEquals(
    const T &expected, const T &actual,
    CppUnit::SourceLine sourceLine,
    const std::string &message,
    const std::string &shortDescription)
{
    if (!CppUnit::assertion_traits<T>::equal(expected, actual))
    {
        CppUnit::Asserter::failNotEqual(
            CppUnit::assertion_traits<T>::toString(expected),
            CppUnit::assertion_traits<T>::toString(actual),
            sourceLine,
            message,
            shortDescription);
    }
}

/**
 * Mixin that can be used to provide the @c equal function for
 * @c CppUnit::assertion_traits.
 */
template<typename T>
class AssertionTraitsEqual
{
public:
    static bool equal(const T &x, const T &y)
    {
        // Allow identical NaNs to compare equal
        if (std::numeric_limits<T>::has_quiet_NaN
            && std::memcmp(&x, &y, sizeof(T)) == 0)
            return true;
        return x == y;
    }
};

template<typename T, std::size_t N>
class AssertionTraitsEqual<std::array<T, N> >
{
public:
    static bool equal(const std::array<T, N> &x, const std::array<T, N> &y)
    {
        for (std::size_t i = 0; i < N; i++)
        {
            if (!AssertionTraitsEqual<T>::equal(x[i], y[i]))
                return false;
        }
        return true;
    }
};

/**
 * Compares two containers for equality.
 */
template<typename T>
void assertContainersEqual(
    const T &expected, const T &actual,
    CppUnit::SourceLine sourceLine, const std::string &message)
{
    typedef typename T::value_type value_type;
    typedef typename T::const_iterator iterator;
    iterator p = std::begin(expected), q = std::begin(actual);
    std::size_t idx = 0;
    while (p != std::end(expected) && q != std::end(actual))
    {
        assertEquals(*p, *q, sourceLine, message,
                     "equality assertion failed at element " + std::to_string(idx));
        ++p;
        ++q;
        ++idx;
    }

    std::size_t expectedSize = idx + std::distance(p, std::end(expected));
    std::size_t actualSize = idx + std::distance(q, std::end(actual));
    assertEquals(expectedSize, actualSize, sourceLine, message,
        "equality assertion failed: container sizes differ");
}

#define UTS_ASSERT_CONTAINERS_EQUAL_MESSAGE(message, expected, actual) \
    (detail::assertContainersEqual( (expected), (actual), CPPUNIT_SOURCELINE(), (message) ))
#define UTS_ASSERT_CONTAINERS_EQUAL(expected, actual) \
    (detail::assertContainersEqual( (expected), (actual), CPPUNIT_SOURCELINE(), ""))

} // namespace detail

namespace CppUnit
{

template<typename T, std::size_t N>
class assertion_traits<std::array<T, N> > : public detail::AssertionTraitsEqual<std::array<T, N> >
{
public:
    static std::string toString(const std::array<T, N> &x)
    {
        std::ostringstream o;
        o << "(" << x[0];
        for (std::size_t i = 1; i < N; i++)
            o << ", " << x[i];
        o << ")";
        return o.str();
    }
};

template<typename A, typename B>
class assertion_traits<std::pair<A, B> > : public detail::AssertionTraitsEqual<std::pair<A, B> >
{
public:
    static std::string toString(const std::pair<A, B> &x)
    {
        std::ostringstream o;
        o << '(' << x.first << ", " << x.second << ')';
        return o.str();
    }
};



} // namespace CppUnit

#endif /* !UTS_TEST_TESTUTIL_H */
