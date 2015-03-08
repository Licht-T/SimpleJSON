SimpleJSON
======================
Simple JSON serializer in C++11.

Usage
------
### Sample Code ###
***simplejson-test.cpp***
```cpp
#include <iostream>
#include <string>
#include <complex>

#include "simplejson.hpp"

// Easy to extend JSON formats for your own classes.
namespace SimpleJSON
{
	template<> inline std::string to_string(const std::complex<double>& c)
	{
		std::ostringstream oss;
		oss << R"({ "complex": true,)";
		oss << R"( "real": )" << c.real() << ",";
		oss << R"( "imag": )" << c.imag() << "}";

		return oss.str();
	}
}

int main(void){
	//Output as array.
	SimpleJSON::JSONStream js("test.json");

	SimpleJSON::JSONObject obj
	{
		{"foo",
			{
				{"abc", 1.23},
				{"def", 456}
			}
		},
		{"bar", SimpleJSON::JSONArray{1, 2, 3}}
	};
	std::cout << obj << std::endl;
	js << obj;

	obj["foo"][""]["123"]=123;
	std::cout << obj << std::endl;
	js << obj;

	obj["bar"][1]=1000.0;
	std::cout << obj << std::endl;
	js << obj;

	(*obj["bar"].getRawArray())[1]=-1;
	std::cout << obj << std::endl;
	js << obj;

	std::complex<double> c(1.0,2.0);
	obj["bar"][1]=c;
	std::cout << obj << std::endl;
	js << obj;

	//Null object
	obj["bar"][1]=SimpleJSON::null;
	std::cout << obj << std::endl;
	js << obj;

	return 0;
}
```

### Compile ###
	g++ -std=c++11 simplejson-test.cpp

License
----------
Copyright &copy; 2015 SimpleJSON
Distributed under the [MIT License][mit].

[Apache]: http://www.apache.org/licenses/LICENSE-2.0
[MIT]: http://www.opensource.org/licenses/mit-license.php
[GPL]: http://www.gnu.org/licenses/gpl.html
