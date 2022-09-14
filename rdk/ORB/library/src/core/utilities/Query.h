/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orb {
/**
 * @brief orb::Query
 *
 * Representation of programme metadata queries.
 */
class Query : public std::enable_shared_from_this<Query> {
public:

    /**
     * Operation enumeration
     */
    enum Operation
    {
        OP_INVALID = -1,
        OP_ID = 0,
        OP_AND,
        OP_OR,
        OP_NOT
    };

    /**
     * Comparison enumeration
     */
    enum Comparison
    {
        CMP_INVALID = -1,
        CMP_EQUAL = 0,
        CMP_NOT_EQL,
        CMP_MORE,
        CMP_MORE_EQL,
        CMP_LESS,
        CMP_LESS_EQL,
        CMP_CONTAINS // case-insensitive string matches
    };

    Query(json in);
    Query(std::string query);

    ~Query();

    std::shared_ptr<Query> And(Query *operator2);
    std::shared_ptr<Query> Or(Query *operator2);
    std::shared_ptr<Query> Not();

    int DescribeContents() const;
    int GetQueryId() const;
    Operation GetOperation() const;
    std::shared_ptr<Query> GetOperator1() const;
    std::shared_ptr<Query> GetOperator2() const;
    std::string GetField() const;
    Comparison GetComparison() const;
    std::string GetValue() const;

    std::string ToString();

private:

    void CommonQuery(json in);
    std::string GetCompareString() const;

    int m_queryId;
    Operation m_operation;
    std::shared_ptr<Query> m_operator1;
    std::shared_ptr<Query> m_operator2;
    std::string m_field;
    Comparison m_comparison;
    std::string m_value;
}; // class Query
} // namespace orb
