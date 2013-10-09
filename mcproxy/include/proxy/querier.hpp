/*
 * This file is part of mcproxy.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
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

/**
 * @addtogroup mod_proxy Proxy
 * @{
 */

#ifndef QUERIER_HPP
#define QUERIER_HPP

#include "include/proxy/membership_db.hpp"
#include "include/proxy/sender.hpp"
#include "include/proxy/timers_values.hpp"
#include "include/proxy/timing.hpp"

#include <functional>
#include <string>
#include <memory>
//#include <chrono>

class proxy_instance;
/**
 * @brief define the behaviour of a multicast querier for a specific interface
 */
class querier
{
private:
    proxy_instance* m_proxy_instance; 
    int m_addr_family;
    int m_if_index;
    membership_db m_db;
    timers_values m_timers_values;

    const std::shared_ptr<const sender> m_sender;
    const std::shared_ptr<timing> m_timing;

    bool init_db();

    //join all router groups or leave them
    bool router_groups_function(std::function<bool(const sender&, int, addr_storage)> f) const;
    void receive_record_in_include_mode(mcast_addr_record_type record_type, const addr_storage& gaddr, source_list<source>& saddr_list, int report_version, gaddr_info& db_info);
    void receive_record_in_exclude_mode(mcast_addr_record_type record_type, const addr_storage& gaddr, source_list<source>& saddr_list, int report_version, gaddr_info& db_info);

    std::shared_ptr<filter_timer> mali(const addr_storage& gaddr) const; //Multicast Address Listener Interval
public:

    virtual ~querier();

    querier(proxy_instance* pr_i, int addr_family, int if_index, const std::shared_ptr<const sender>& sender, const std::shared_ptr<timing>& timing);

    void receive_record(mcast_addr_record_type record_type, const addr_storage& gaddr, source_list<source>& saddr_list, int report_version);

    void timer_triggerd(const std::shared_ptr<timer_msg>& msg);

    void receive_query();

    timers_values& get_timers_values();
    std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& stream, const querier& q);

    /**
     * @brief Test the functionality of the querier.
     */
};

#endif // QUERIER_HPP
/** @} */
