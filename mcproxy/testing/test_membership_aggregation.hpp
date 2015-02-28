/*
 * This file is part of mcproxy.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * written by Sebastian Woelke, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * Website: http://mcproxy.realmv6.org/
 */

#ifndef  TEST_MEMBERSHIP_AGGREGATION_HPP
#define  TEST_MEMBERSHIP_AGGREGATION_HPP

#ifdef UNIT_TESTS

#include "testing/ut_bootstrap.hpp"
#include "testing/ut_suite.hpp"
#include "include/proxy/simple_membership_aggregation.hpp"
#include "include/proxy/message_format.hpp"


struct test_membership_aggregation {

    test_status test_merge_reminder_disjoint_fun() {
        HC_LOG_TRACE("");
        UT_INITIALISATION;

        using mss = mem_source_state;
        using fss = filter_source_state;
        using sl = source_list<source>;

        const addr_storage s0("0.0.0.0");
        const addr_storage s1("1.1.1.1");
        const addr_storage s2("2.2.2.2");
        const addr_storage s3("3.3.3.3");
        const mss in_a(INCLUDE_MODE, sl {
            s1, s2
        });
        const mss ex_a(EXCLUDE_MODE, sl {
            s1, s2
        });
        const mss in_b(INCLUDE_MODE, sl {
            s1, s3
        });
        const mss ex_b(EXCLUDE_MODE, sl {
            s1, s3
        });

        const fss wl_b(FT_WHITELIST, sl {
            s1, s3
        });
        const fss bl_b(FT_BLACKLIST, sl {
            s1, s3
        });
        const fss wl_wc(FT_WHITELIST, sl {
            s0
        });
        const fss bl_wc(FT_BLACKLIST, sl {
            s0
        });

        simple_membership_aggregation s_mem_agg(IGMPv3);

        auto check_merge_convert_wildcard_filter = [&](const fss & convert, const fss & result) {
            fss to_tmp = convert;
            s_mem_agg.convert_wildcard_filter(to_tmp);

            if (to_tmp == result) {
                return true;
            } else {
                return false;
            }
        };

        UT_CHECK(check_merge_convert_wildcard_filter(wl_b, wl_b));
        UT_CHECK(check_merge_convert_wildcard_filter(bl_b, bl_b));
        UT_CHECK(check_merge_convert_wildcard_filter(wl_wc, fss(FT_BLACKLIST, sl {})));

        UT_CHECK(check_merge_convert_wildcard_filter(bl_wc, fss(FT_WHITELIST, sl {})));
        UT_CHECK(check_merge_convert_wildcard_filter(fss(FT_WHITELIST, sl {s1, s2, s0, s3}), fss(FT_BLACKLIST, sl {})));
        UT_CHECK(check_merge_convert_wildcard_filter(fss(FT_BLACKLIST, sl {s1, s2, s0, s3}), fss(FT_WHITELIST, sl {})));


        auto check_merge_group_memberships = [&](const mss & to, const mss & from, const mss & result) {
            mss to_tmp = to;
            s_mem_agg.merge_group_memberships(to_tmp, from);

            if (to_tmp == result) {
                return true;
            } else {
                return false;
            }
        };

        //IN{s1,s2} merge with IN{s1,s3} = IN{s1,s2,s3}
        UT_CHECK(check_merge_group_memberships(in_a, in_b, mss(INCLUDE_MODE, sl {s1, s2, s3})));

        //IN{s1,s2} merge with EX{s1,s3} = EX{s3}
        UT_CHECK(check_merge_group_memberships(in_a, ex_b, mss(EXCLUDE_MODE, sl {s3})));

        //EX{s1,s2} merge with IN{s1,s3} = EX{s2}
        UT_CHECK(check_merge_group_memberships(ex_a, in_b, mss(EXCLUDE_MODE, sl {s2})));

        //EX{s1,s2} merge with EX{s1,s3} = EX{s1}
        UT_CHECK(check_merge_group_memberships(ex_a, ex_b, mss(EXCLUDE_MODE, sl {s1})));

        auto check_merge_memberships_filter = [&](const mss & to, const fss & from, const mss & expected_result) {
            mss to_result_tmp = to;
            s_mem_agg.merge_memberships_filter(to_result_tmp, from);

            if (to_result_tmp == expected_result) {
                return true;
            } else {
                return false;
            }
        };

        //IN{s1,s2} merge with WL{s1,s3} = IN{s1} R(IN{s2})
        UT_CHECK(check_merge_memberships_filter(in_a, wl_b, mss(INCLUDE_MODE, sl {s1})));
        //IN{s1,s2} merge with BL{s1,s3} = IN{s2} R(IN{s1})
        UT_CHECK(check_merge_memberships_filter(in_a, bl_b, mss(INCLUDE_MODE, sl {s2})));
        //EX{s1,s2} merge with WL{s1,s3} = IN{s3} R(EX{s1,s2,s3})
        UT_CHECK(check_merge_memberships_filter(ex_a, wl_b, mss(INCLUDE_MODE, sl {s3})));
        //EX{s1,s2} merge with BL{s1,s3} = EX{s1,s2,s3} R(IN{s3})
        UT_CHECK(check_merge_memberships_filter(ex_a, bl_b, mss(EXCLUDE_MODE, sl {s1, s2, s3})));


        //IN{s1,s2} merge with WL{*} = IN{s1,s2} R(IN{})
        UT_CHECK(check_merge_memberships_filter(in_a, wl_wc, mss(INCLUDE_MODE, sl {s1, s2})));
        //IN{s1,s2} merge with BL{*} = IN{} R(IN{s1,s2})
        UT_CHECK(check_merge_memberships_filter(in_a, bl_wc, mss(INCLUDE_MODE, sl {})));
        //EX{s1,s2} merge with WL{*} = EX{s1,s2} R(IN{})
        UT_CHECK(check_merge_memberships_filter(ex_a, wl_wc, mss(EXCLUDE_MODE, sl {s1, s2})));
        //EX{s1,s2} merge with BL{*} = IN{} R(EX{s1,s2})
        UT_CHECK(check_merge_memberships_filter(ex_a, bl_wc, mss(INCLUDE_MODE, sl {})));


        auto check_merge_memberships_filter_reminder = [&](const mss & to, const fss & from, const mss & expected_result, const mss & expected_reminder) {
            mss to_result_tmp = to;
            s_mem_agg.merge_memberships_filter(to_result_tmp, from);

            if (to_result_tmp != expected_result) {
                return false;
            }

            mss to_reminder_tmp = to;
            s_mem_agg.merge_memberships_filter_reminder(to_reminder_tmp, to_result_tmp, from);

            if (to_reminder_tmp == expected_reminder) {
                return true;
            } else {
                return false;
            }
        };

        //IN{s1,s2} merge with WL{s1,s3} = IN{s1} R(IN{s2})
        UT_CHECK(check_merge_memberships_filter_reminder(in_a, wl_b, mss(INCLUDE_MODE, sl {s1}), mss(INCLUDE_MODE, sl {s2})));
        //IN{s1,s2} merge with BL{s1,s3} = IN{s2} R(IN{s1})
        UT_CHECK(check_merge_memberships_filter_reminder(in_a, bl_b, mss(INCLUDE_MODE, sl {s2}), mss(INCLUDE_MODE, sl {s1})));
        //EX{s1,s2} merge with WL{s1,s3} = IN{s3} R(EX{s1,s2,s3})
        UT_CHECK(check_merge_memberships_filter_reminder(ex_a, wl_b, mss(INCLUDE_MODE, sl {s3}), mss(EXCLUDE_MODE, sl {s1, s2, s3})));
        //EX{s1,s2} merge with BL{s1,s3} = EX{s1,s2,s3} R(IN{s3})
        UT_CHECK(check_merge_memberships_filter_reminder(ex_a, bl_b, mss(EXCLUDE_MODE, sl {s1, s2, s3}), mss(INCLUDE_MODE, sl {s3})));


        //IN{s1,s2} merge with WL{*} = IN{s1,s2} R(IN{})
        UT_CHECK(check_merge_memberships_filter_reminder(in_a, wl_wc, mss(INCLUDE_MODE, sl {s1, s2}), mss(INCLUDE_MODE, sl {})));
        //IN{s1,s2} merge with BL{*} = IN{} R(IN{s1,s2})
        UT_CHECK(check_merge_memberships_filter_reminder(in_a, bl_wc, mss(INCLUDE_MODE, sl {}), mss(INCLUDE_MODE, sl {s1, s2})));
        //EX{s1,s2} merge with WL{*} = EX{s1,s2} R(IN{})
        UT_CHECK(check_merge_memberships_filter_reminder(ex_a, wl_wc, mss(EXCLUDE_MODE, sl {s1, s2}), mss(INCLUDE_MODE, sl {})));
        //EX{s1,s2} merge with BL{*} = IN{} R(EX{s1,s2})
        UT_CHECK(check_merge_memberships_filter_reminder(ex_a, bl_wc, mss(INCLUDE_MODE, sl {}), mss(EXCLUDE_MODE, sl {s1, s2})));


        auto check_disjoin_group_memberships = [&](const mss & to, const mss & from, const mss & expected_result) {
            mss to_result_tmp = to;
            s_mem_agg.disjoin_group_memberships(to_result_tmp, from);

            if (to_result_tmp == expected_result) {
                return true;
            } else {
                return false;
            }
        };

        UT_CHECK(check_disjoin_group_memberships(in_a, in_b, mss(INCLUDE_MODE, sl {s2})));
        UT_CHECK(check_disjoin_group_memberships(in_a, ex_b, mss(INCLUDE_MODE, sl {s1})));
        UT_CHECK(check_disjoin_group_memberships(ex_a, in_b, mss(EXCLUDE_MODE, sl {s1,s2,s3})));
        UT_CHECK(check_disjoin_group_memberships(ex_a, ex_b, mss(INCLUDE_MODE, sl {s2})));

        UT_SUMMARY;
    }


    test_status test_mem_aggregation() {
        HC_LOG_TRACE("");
        UT_INITIALISATION;





        UT_SUMMARY;
    }
};

std::list<std::tuple<ut_test_fun, ut_effort>> test_membership_aggregation_functions()
{
    HC_LOG_TRACE("");
    test_membership_aggregation p;

    return std::list<std::tuple<ut_test_fun, ut_effort>> {
        std::make_tuple([&p]() {
            return p.test_merge_reminder_disjoint_fun();
        }, 1)
    };
}


#endif //UNIT_TESTS

#endif //TEST_MEMBERSHIP_AGGREGATION_HPP
