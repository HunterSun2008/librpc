#ifndef RPC_SERVER_H
#define RPC_SERVER_H

#include "std.hpp"
#include "utils.hpp"
#include "jsonrp.hpp"

namespace rpc
{

struct server //: public server_base
{
    static std::shared_ptr<server> create(unsigned short port);

    virtual ~server(){};

    //server(const server& s) = delete;
    //server& operator=(const server& s) = delete;

    //server(const server&& s);
    //server& operator=(const server&& s);

    virtual bool run() = 0;

    virtual void stop() = 0;

    template <typename TFunc>
    void add_handler(std::string name, TFunc func)
    {
        if (m_handlers.find(name) != std::end(m_handlers))
        {
            throw std::invalid_argument{"[" + std::string{__func__} + "] Failed to add handler. "
                                                                      "The function \"" +
                                        name + "\" already exists."};
        }

        auto wrapper = [f = std::move(func)](std::string request) -> Json {
            std::function func{std::move(f)};

            using function_meta = utils::detail::function_meta<decltype(func)>;
            using arguments_tuple_type = typename function_meta::arguments_tuple_type;
            //using return_type = typename function_meta::return_type;

            arguments_tuple_type arguments;
            std::istringstream iss(request);

            std::apply([&](auto &... args) { ((iss >> args), ...); }, arguments);

            auto ret = std::apply(std::move(func), std::move(arguments));
            
            Json result;
                        
            result = ret;

            return result;
        };

        m_handlers.emplace(std::move(name), std::move(wrapper));
    }

    template <typename... T>
    auto call(std::string name, T... args)
    {
        std::ostringstream oss;

        ((oss << args << " "), ...);

        auto iter = m_handlers.find(name);

        if (iter != std::end(m_handlers))
        {
            return iter->second(oss.str());
        }
    }

    Json exec(const std::string &name, const std::string &serialization)
    {
        auto iter = m_handlers.find(name);

        if (iter != std::end(m_handlers))
        {
            return iter->second(serialization);
        }
        else
            throw std::runtime_error("method not found");        
    }

  private:
    typedef std::function<Json(std::string)> handler_type;
    std::map<std::string, handler_type> m_handlers;
};

typedef std::shared_ptr<server> server_ptr;

} // namespace rpc

#endif