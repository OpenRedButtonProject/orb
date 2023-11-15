/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ORBLogging.h"
#include "Query.h"

namespace orb {
/**
 * Constructor.
 *
 * @param in
 */
Query::Query(json in)
    : m_queryId(-1)
    , m_operation(Operation::OP_INVALID)
    , m_operator1(nullptr)
    , m_operator2(nullptr)
    , m_field("")
    , m_comparison(Comparison::CMP_INVALID)
    , m_value("")
{
    CommonQuery(in);
}

/**
 * Constructor.
 *
 * @param query
 */
Query::Query(std::string query)
    : m_queryId(-1)
    , m_operation(Operation::OP_INVALID)
    , m_operator1(nullptr)
    , m_operator2(nullptr)
    , m_field("")
    , m_comparison(Comparison::CMP_INVALID)
    , m_value("")
{
    json in = json::parse(query);
    CommonQuery(in);
}

/**
 * Destructor.
 */
Query::~Query()
{
}

/**
 * Query::And
 *
 * @param operator2
 *
 * @return
 */
std::shared_ptr<Query> Query::And(Query *operator2)
{
    m_operation = Operation::OP_AND;
    std::shared_ptr<Query> operator2_shared_ptr(operator2);
    m_operator2 = operator2_shared_ptr;
    return shared_from_this();
}

/**
 * Query::Or
 *
 * @param operator2
 *
 * @return
 */
std::shared_ptr<Query> Query::Or(Query *operator2)
{
    m_operation = Operation::OP_OR;
    std::shared_ptr<Query> operator2_shared_ptr(operator2);
    m_operator2 = operator2_shared_ptr;
    return shared_from_this();
}

/**
 * Query::Not
 *
 * @return
 */
std::shared_ptr<Query> Query::Not()
{
    m_operation = Operation::OP_NOT;
    return shared_from_this();
}

/**
 * Query::DescribeContents
 *
 * @return
 */
int Query::DescribeContents() const
{
    return 0;
}

/**
 * Query::GetQueryId
 *
 * @return
 */
int Query::GetQueryId() const
{
    return m_queryId;
}

/**
 * Query::GetOperation
 *
 * @return
 */
Query::Operation Query::GetOperation() const
{
    return m_operation;
}

/**
 * Query::GetOperator1
 *
 * @return
 */
std::shared_ptr<Query> Query::GetOperator1() const
{
    return m_operator1;
}

/**
 * Query::GetOperator2
 *
 * @return
 */
std::shared_ptr<Query> Query::GetOperator2() const
{
    return m_operator2;
}

/**
 * Query::GetField
 *
 * @return
 */
std::string Query::GetField() const
{
    return m_field;
}

/**
 * Query::GetComparison
 *
 * @return
 */
Query::Comparison Query::GetComparison() const
{
    return m_comparison;
}

/**
 * Query::GetValue
 *
 * @return
 */
std::string Query::GetValue() const
{
    return m_value;
}

/**
 * Query::ToString
 *
 * @return A string representation of the Query object
 */
std::string Query::ToString()
{
    std::string result;
    if (m_queryId != -1)
    {
        result += "Query_";
        result += std::to_string(m_queryId);
        result += " ";
    }
    result += "(";
    switch (m_operation)
    {
        case Operation::OP_AND:
        {
            if (m_operator1 != nullptr)
            {
                result += m_operator1->ToString();
            }
            result += ".AND.";
            if (m_operator2 != nullptr)
            {
                result += m_operator2->ToString();
            }
            break;
        }
        case Operation::OP_OR:
        {
            if (m_operator1 != nullptr)
            {
                result += m_operator1->ToString();
            }
            result += ".OR.";
            if (m_operator2 != nullptr)
            {
                result += m_operator2->ToString();
            }
            break;
        }
        case Operation::OP_NOT:
        {
            if (m_operator1 != nullptr)
            {
                result += m_operator1->ToString();
            }
            result += ".NOT.";
            break;
        }
        case Operation::OP_ID:
            result += m_field + GetCompareString() + "'" + m_value + "'";
            break;
        default:
            break;
    } // switch
    result += ")";
    return result;
}

/**
 * Query::CommonQuery
 *
 * @param in
 */
void Query::CommonQuery(json in)
{
    m_comparison = Comparison::CMP_INVALID;
    m_queryId = in.value("queryId", -1);

    std::string operation = in.value("operation", "");
    if (operation == "IDENTITY")
    {
        m_operation = Operation::OP_ID;
    }
    else if (operation == "AND")
    {
        m_operation = Operation::OP_AND;
    }
    else if (operation == "OR")
    {
        m_operation = Operation::OP_OR;
    }
    else if (operation == "NOT")
    {
        m_operation = Operation::OP_NOT;
    }
    else
    {
        m_operation = Operation::OP_ID;
    }

    json arguments = {};
    if (in.contains("arguments") && in["arguments"].is_array())
    {
        arguments = in["arguments"];
    }

    if (!arguments.empty())
    {
        json arg1 = arguments[0];
        m_operator1 = std::make_shared<Query>(arg1);
        if (arguments.size() > 1)
        {
            json arg2 = arguments[1];
            m_operator2 = std::make_shared<Query>(arg2);
        }
    }

    if (m_operation == Operation::OP_ID)
    {
        m_field = in["field"];
        m_comparison = static_cast<Comparison>(in["comparison"]);

        if (in["value"].is_number())
        {
            int value = in["value"];
            m_value = std::to_string(value);
        }
        else
        {
            m_value = in["value"];
        }
    }
}

/**
 * Query::GetCompareString
 *
 * @return
 */
std::string Query::GetCompareString() const
{
    switch (m_comparison)
    {
        case Comparison::CMP_INVALID:
            return "";
        case Comparison::CMP_EQUAL:
            return " == ";
        case Comparison::CMP_NOT_EQL:
            return " != ";
        case Comparison::CMP_MORE:
            return " > ";
        case Comparison::CMP_MORE_EQL:
            return " >= ";
        case Comparison::CMP_LESS:
            return " < ";
        case Comparison::CMP_LESS_EQL:
            return " <= ";
        case Comparison::CMP_CONTAINS:
            return " Ct ";
        default:
            return "";
    } // switch
    return "";
}
} // namespace orb
