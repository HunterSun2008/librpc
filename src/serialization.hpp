#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP
#include "jsonrp.hpp"

template <typename>
struct is_pair : std::false_type
{
};

template <typename T, typename U>
struct is_pair<std::pair<T, U>> : std::true_type
{
};

template <class ITR, typename = typename std::enable_if<is_pair<typename ITR::value_type>::value, ITR>::type>
decltype(auto) do_stuff(ITR &&itr)
{
    //access of itr->second ok.
}

namespace rpc
{
class json_request
{
  public:
    json_request(uint32_t id, const std::string &method) : m_request(id, method, Json())
    {
    }

    template <typename T>
    json_request &operator<<(const T &t)
    {
        if constexpr (is_pair<T>::value)
            m_params[t.first] = t.second;
        else
            m_params.push_back(t);

        return *this;
    }

    std::string to_string()
    {
        m_request.params = m_params;
        return m_request.to_json().dump();
    }

    template<typename T>
    json_request & operator >> (T & t)
    {
        //m_params.get
        return *this;
    }

    uint32_t id()
    {
        return m_request.id.int_id;
    }

  private:
    jsonrpcpp::Request m_request;
    Json m_params;
};

class json_response
{
    public:
        json_response(const std::string & stream)
        {
            jsonrpcpp::Parser parse;
            jsonrpcpp::entity_ptr entity = parse.parse(stream);
            if(entity && entity->is_response())
            {
                jsonrpcpp::response_ptr response = std::dynamic_pointer_cast<jsonrpcpp::Response>(entity);

                m_result = response->result.dump();
            }
            
        }
    private:
        std::string m_result;
};
} // namespace rpc
#endif