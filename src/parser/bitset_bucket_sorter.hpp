#ifndef BITSET_BUCKET_SORTER
#define BITSET_BUCKET_SORTER

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 50
#define BOOST_MPL_LIMIT_MAP_SIZE 50
#include <boost/mpl/vector_c.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/accumulate.hpp>
#include <boost/mpl/long.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/shift_left.hpp>
#include <boost/mpl/shift_right.hpp>
#include <boost/mpl/bitor.hpp>
#include <boost/mpl/modulus.hpp>
#include <boost/mpl/less.hpp>
#include <boost/mpl/times.hpp>
#include <boost/mpl/erase_key.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/advance.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/at.hpp>

template<typename seq, typename key, typename item>
struct replace_pos {


    typedef typename boost::mpl::erase_key<seq, key>::type temp;
    typedef typename boost::mpl::insert<temp, boost::mpl::pair<key, item> >::type type;

};

// mutli-pass bucket sort
template<typename token_set, int bucket_number, int limit>
struct bitset_bucket {

    typedef typename boost::mpl::if_<boost::mpl::less<boost::mpl::times<boost::mpl::long_<bucket_number>, boost::mpl::int_<32> >, boost::mpl::long_<limit> >,
            typename boost::mpl::accumulate<token_set, boost::mpl::long_<0>,
                boost::mpl::if_<
                    boost::mpl::equal_to<boost::mpl::shift_right<boost::mpl::_2, boost::mpl::long_<5> >, boost::mpl::long_<bucket_number> >,
                    boost::mpl::bitor_<boost::mpl::_1, boost::mpl::shift_left<boost::mpl::long_<1>, boost::mpl::modulus<boost::mpl::_2, boost::mpl::long_<32> > > >,
                    boost::mpl::_1 >
                >::type,
            boost::mpl::long_<0> >::type type;

};

/*
template<typename token_set>
struct bitset_buckets {

    static const int num_token_longs = 24;
    static const unsigned long data[num_token_longs];

};


template<typename token_set>
const unsigned long bitset_buckets<token_set>::data[bitset_buckets<token_set>::num_token_longs] = 
        { bitset_bucket<token_set, 0>::type::value, bitset_bucket<token_set, 1>::type::value, bitset_bucket<token_set, 2>::type::value, bitset_bucket<token_set, 3>::type::value
        , bitset_bucket<token_set, 4>::type::value, bitset_bucket<token_set, 5>::type::value, bitset_bucket<token_set, 6>::type::value, bitset_bucket<token_set, 7>::type::value
        , bitset_bucket<token_set, 8>::type::value, bitset_bucket<token_set, 9>::type::value, bitset_bucket<token_set, 10>::type::value, bitset_bucket<token_set, 11>::type::value
        , bitset_bucket<token_set, 12>::type::value, bitset_bucket<token_set, 13>::type::value, bitset_bucket<token_set, 14>::type::value, bitset_bucket<token_set, 15>::type::value
        , bitset_bucket<token_set, 16>::type::value, bitset_bucket<token_set, 17>::type::value, bitset_bucket<token_set, 18>::type::value, bitset_bucket<token_set, 19>::type::value
        , bitset_bucket<token_set, 20>::type::value, bitset_bucket<token_set, 21>::type::value, bitset_bucket<token_set, 22>::type::value, bitset_bucket<token_set, 23>::type::value
};
*/

template<typename token_set>
struct bitset_bucket_sorter {

    typedef typename boost::mpl::accumulate<token_set, boost::mpl::map<
                                            boost::mpl::pair<boost::mpl::integral_c<long, 0>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 1>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 2>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 3>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 4>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 5>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 6>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 7>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 8>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 9>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 10>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 11>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 12>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 13>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 14>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 15>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 16>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 17>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 18>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 19>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 20>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 21>, boost::mpl::integral_c<long, 0> >,
                                            boost::mpl::pair<boost::mpl::integral_c<long, 22>, boost::mpl::integral_c<long, 0> >, boost::mpl::pair<boost::mpl::integral_c<long, 23>, boost::mpl::integral_c<long, 0> > >,

        replace_pos<boost::mpl::_1, boost::mpl::shift_right<boost::mpl::_2, boost::mpl::integral_c<long, 5> >,
                    boost::mpl::bitor_<
                        boost::mpl::at<boost::mpl::_1, boost::mpl::shift_right<boost::mpl::_2, boost::mpl::integral_c<long, 5> > >,
                        boost::mpl::shift_left<boost::mpl::integral_c<long, 1>, boost::mpl::modulus<boost::mpl::_2, boost::mpl::integral_c<long, 32> > >
                    >
        >
    >::type type;

};


template<typename token_set>
struct bitset_buckets {

    typedef typename bitset_bucket_sorter<token_set>::type buckets;

    static const int num_token_longs = 24;
    static const unsigned long data[num_token_longs];

};

template<typename token_set>
const unsigned long bitset_buckets<token_set>::data[bitset_buckets<token_set>::num_token_longs] = 
{
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 0> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 1> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 2> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 3> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 4> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 5> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 6> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 7> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 8> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 9> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 10> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 11> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 12> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 13> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 14> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 15> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 16> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 17> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 18> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 19> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 20> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 21> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 22> >::type::value,
    boost::mpl::at<bitset_buckets<token_set>::buckets, boost::mpl::integral_c<long, 23> >::type::value

};

#endif