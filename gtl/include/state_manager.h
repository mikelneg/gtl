#ifndef HBIWOAJBVSF_GTL_STATE_MANAGER_H_
#define HBIWOAJBVSF_GTL_STATE_MANAGER_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::
    
    class state_manager;
-----------------------------------------------------------------------------*/

#include <gtl/include/stage.h>
#include <gtl/include/events.h>
#include <boost/coroutine/asymmetric_coroutine.hpp>

namespace gtl {

    namespace detail { 
            [&](evco::pull_type& yield){                        
            while (!same_type(yield.get(),gtl::events::sendquit{})){
                std::cout << "entering state..\n";
                //int result = boost::apply_visitor([&](auto& s){ return s(yield); }, stayyt);                
                auto visit = vn::make_lambda_visitor<int>([&](auto& v){  return v(yield); }, [](boost::blank){ return 0; });
                int result = boost::apply_visitor(visit,stayyt); // drop to stayyt..                                                    
                std::cout << "state exited..\n";
                if (!state_manager(yield, result)) { return; }                
            }
    }




class state_manager {
    
    using coroutine = boost::coroutines::asymmetric_coroutine<gtl::event>;        

    coroutine dispatcher;    

public:
    
    state_manager() 
        : dispatcher{} 
    {}

         evco::push_type handler{[&](evco::pull_type& yield){                        
            while (!same_type(yield.get(),gtl::events::sendquit{})){
                std::cout << "entering state..\n";
                //int result = boost::apply_visitor([&](auto& s){ return s(yield); }, stayyt);                
                auto visit = vn::make_lambda_visitor<int>([&](auto& v){  return v(yield); }, [](boost::blank){ return 0; });
                int result = boost::apply_visitor(visit,stayyt); // drop to stayyt..                                                    
                std::cout << "state exited..\n";
                if (!state_manager(yield, result)) { return; }                
            }
        }};

                state_v stayyt = state_A{};

        auto state_manager = [&](evco::pull_type& yield, int result) {
              using boost::get;      
              if (result == 1) { 
                  std::cout << "state_manager swapping states..\n";  
                  //state_v transit = transition_h{std::move(stayyt), state_v{state_B{}}};
                  using std::swap;                
                  stayyt = transition_h{std::move(stayyt), state_v{state_B{}}}; //swap(stayyt, transit);
                  auto visit = vn::make_lambda_visitor([&](auto& v){ v(yield); }, [](boost::blank){});
                  boost::apply_visitor(visit,stayyt); // drop to stayyt..                                  
                  stayyt = state_B{std::move(boost::get<state_B>(boost::get<transition_h>(stayyt).s2))};
                  //swap(boost::get<transition_h>(stayyt).s2, stayyt);
              }
              if (result == 2) { std::cout << "state_manager bonking out..\n"; return false; }
              return true;
        };  

};
  
} // namespaces
#endif
