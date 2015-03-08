#pragma once

#include <iostream>
#include <fstream>
#include <type_traits>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <utility>
#include <initializer_list>
#include <cstdlib>
#include <string>

#include <complex>

/**
 * @brief SimpleJSON namespace.
 */
namespace SimpleJSON
{
	struct json_null{};
	/**
	 * @brief JSON null.
	 */
	struct json_null null;
	template<class T> struct typeInfo { static char p; };
	template<class T> char typeInfo<T>::p;
	typedef void* type_id;
	template<class T> constexpr type_id get_type_id() { return &typeInfo<T>::p; }

	/**
	 * @brief Output JSON string of type T.<br>
	 * This is the forward declaration template.<br>
	 * Specialize this template as you want.<br>
	 *
	 * @tparam T Type.
	 * @param t Element.
	 *
	 * @return JSON string.
	 */
	template <typename T> inline std::string to_string(const T& t);

	inline const std::string& to_string(const struct json_null& n){
		static const std::string nullstr("null");
		return nullstr;
	}

	inline const std::string& to_string(const bool& x)
	{
		static const std::string truestr("true"), falsestr("false");
		if(x)
		{
			return truestr;
		}
		else
		{
			return falsestr;
		}
	}

	template <typename T> inline std::string to_string(const std::vector<T>& vec)
	{
		std::ostringstream oss;
		oss << "[";
		if (!vec.empty())
		{
			std::copy(vec.cbegin(), vec.cend()-1, std::ostream_iterator<T>(oss, ","));
			oss << vec.back();
		}
		oss << "]";
		return oss.str();
	}

	template <typename T> inline std::string to_string(const std::map<std::string, T>& map)
	{
		std::ostringstream oss;
		oss << "{";
		if (!map.empty())
		{
			unsigned int size=1;
			const unsigned int maxsize=map.size();
			for(auto bitr=map.cbegin(), eitr=map.cend(); bitr!=eitr; bitr++, size++)
			{
				oss << "\"" << bitr->first << "\": ";
				oss << bitr->second;
				if(size<maxsize){
					oss << ", ";
				}
			}
		}
		oss << "}";
		return oss.str();
	}

	/**
	 * @brief JSON element class.<br>
	 * It can contain any type/class object.
	 */
	class JSONElement
	{
		private:
			class placeholder
			{
				public:
					virtual ~placeholder(){}
					virtual placeholder* clone(void) = 0;
					virtual type_id type(void) const = 0;
					virtual std::string toString(void) = 0;
			};

			template<typename T, typename Enable = void> class holder;

			template<typename T> class holder
				<T, typename std::enable_if< std::is_arithmetic<T>::value && !std::is_same<T,bool>::value >::type> : public placeholder
				{
					public:
						explicit holder(const T& data)
						{
							held=data;
						}
						virtual placeholder* clone(void)
						{
							return new holder(held);
						}
						virtual std::string toString(void)
						{
							return std::to_string(held);
						}
						virtual constexpr type_id type() const
						{
							return get_type_id<T>();
						}
					private:
						T held;
				};

			template<typename T> class holder
				<T, typename std::enable_if< std::is_same<T,std::string>::value >::type> : public placeholder
				{
					public:
						explicit holder(const T& data)
							: held(data)
						{ }
						virtual placeholder* clone(void)
						{
							return new holder(held);
						}
						virtual std::string toString(void)
						{
							return "\""+held+"\"";
						}
						virtual constexpr type_id type() const
						{
							return get_type_id<T>();
						}
					private:
						T held;
				};

			template<typename T> class holder
				<T, typename std::enable_if< std::is_same<T,std::map<std::string,JSONElement>>::value >::type> : public placeholder
				{
					public:
						explicit holder(const T& data)
							: held(data)
						{ }
						virtual placeholder* clone(void)
						{
							return new holder(held);
						}
						virtual std::string toString(void)
						{
							return to_string(held);
						}
						virtual constexpr type_id type() const
						{
							return get_type_id<T>();
						}
						JSONElement& operator [](const std::string& s)
						{
							return held[s];
						}
						T* getRawObject()
						{
							return &held;
						}
					private:
						T held;
				};

			template<typename T> class holder
				<T, typename std::enable_if< std::is_same<T,std::vector<JSONElement>>::value >::type> : public placeholder
				{
					public:
						explicit holder(const T& data)
							: held(data)
						{ }
						virtual placeholder* clone(void)
						{
							return new holder(held);
						}
						virtual std::string toString(void)
						{
							return to_string(held);
						}
						virtual constexpr type_id type() const
						{
							return get_type_id<T>();
						}
						JSONElement& operator [](const int& i)
						{
							return held[i];
						}
						T* getRawArray()
						{
							return &held;
						}
					private:
						T held;
				};

			template<typename T> class holder
				<T, typename std::enable_if<
				!(
						std::is_same<T,std::string>::value ||
						(std::is_arithmetic<T>::value && !std::is_same<T,bool>::value) ||
						std::is_same<T,std::map<std::string,JSONElement>>::value||
						std::is_same<T,std::vector<JSONElement>>::value
				 )
				>::type> : public placeholder
				{
					public:
						explicit holder(const T& data)
						{ held=data; }
						virtual placeholder* clone(void)
						{
							return new holder(held);
						}
						virtual std::string toString(void)
						{
							return to_string(held);
						}
						virtual constexpr type_id type() const
						{
							return get_type_id<T>();
						}
					private:
						T held;
				};

			type_id type() const
			{
				return content ? content->type() : nullptr;
			}

		public:
			template<typename T>
				JSONElement(const T& data)
				: content(new holder<T>(data) )
			{ }
			JSONElement(void)
				: content(new holder<std::map<std::string, JSONElement>>(std::map<std::string, JSONElement>()))
			{ }

			JSONElement(const std::initializer_list<std::pair<const std::string,JSONElement>>& data)
				: content(new holder<std::map<std::string, JSONElement>>(std::map<std::string, JSONElement>(data)))
			{ }

			JSONElement(const JSONElement& other)
				: content(other.content.get() ? other.content->clone() : 0)
			{ }

			JSONElement & operator = (const JSONElement& rhs)
			{
				if (this != &rhs)
				{
					content.reset(rhs.content->clone());
				}
				return *this;
			}

			template <typename T>
				JSONElement & operator = (const T& rhs)
				{
					content.reset(new holder<T>(rhs));
					return *this;
				}

			std::string toString(void) const
			{
				return content->toString();
			}

			/**
			 * @brief Get a json element which key is k on JSON object.
			 *
			 * @param k Key of element.
			 *
			 * @return Reference of a JSON element which key is k if this contains a JSON object.<br>
			 * If not a JSON object, the program exits with a failure status.
			 */
			JSONElement& operator [](const std::string& k)
			{
				if(this->type()==get_type_id<std::map<std::string, JSONElement>>())
				{
					return (*(static_cast<
								holder< std::map<std::string, JSONElement> >*
								>(content.get())))[k];
				}
				else
				{
					std::cerr << "Invalid JSON object reference (Not JSON object)." << std::endl;
					exit(1);
				}
			}

			/**
			 * @brief Get a JSON object as STL map.
			 *
			 * @return Reference of a JSON object as STL map if this contains a JSON object.<br>
			 * If not a JSON object, the program exits with a failure status.
			 */
			std::map<std::string, JSONElement>* getRawObject()
			{
				if(this->type()==get_type_id<std::map<std::string, JSONElement>>())
				{
					return (*(static_cast<
								holder< std::map<std::string, JSONElement> >*
								>(content.get()))).getRawObject();
				}
				else
				{
					std::cerr << "Invalid JSON object reference (Not JSON object)." << std::endl;
					exit(1);
				}
			}

			/**
			 * @brief Get a json element at index i on json array.
			 *
			 * @param i Index.
			 *
			 * @return Reference of a JSON element at index i if this contains a JSON array.<br>
			 * If not a JSON array, the program exits with a failure status.
			 */
			JSONElement& operator [](const int& i)
			{
				if(this->type()==get_type_id<std::vector<JSONElement>>())
				{
					return (*(static_cast<
								holder< std::vector<JSONElement> >*
								>(content.get())))[i];
				}
				else
				{
					std::cerr << "Invalid JSON array reference (Not JSON array)." << std::endl;
					exit(1);
				}
			}

			/**
			 * @brief Get a JSON array as STL vector.
			 *
			 * @return Reference of a JSON array as STL vector if this contains a JSON array.<br>
			 * If not a JSON array, the program exits with a failure status.
			 */
			std::vector<JSONElement>* getRawArray()
			{
				if(this->type()==get_type_id<std::vector<JSONElement>>())
				{
					return (*(static_cast<
								holder< std::vector<JSONElement> >*
								>(content.get()))).getRawArray();
				}
				else
				{
					std::cerr << "Invalid JSON array reference (Not JSON array)." << std::endl;
					exit(1);
				}
			}

		private:
			std::unique_ptr<placeholder> content;
	};

	std::ostream& operator<<(std::ostream& os,const JSONElement& x)
	{
		return (os << x.toString());
	}

	/**
	 * @brief JSON object type.
	 */
	using JSONObject = std::map<std::string, JSONElement>;
	/**
	 * @brief JSON array type.
	 */
	using JSONArray = std::vector<JSONElement>;

	std::ostream& operator<<(std::ostream& os, const JSONObject& x)
	{
		return (os << to_string(x));
	}
	std::ostream& operator<<(std::ostream& os, const JSONArray& x)
	{
		return (os << to_string(x));
	}

	/**
	 * @brief Write JSON into file as array.
	 */
	class JSONStream
	{
		private:
			std::ofstream os;
			bool flag;
		public:
			/**
			 * @brief Constructor.
			 *
			 * @param fname Output file name.
			 */
			JSONStream(const std::string& fname)
				:os(fname)
			{
				flag = false;
				os << "[" << std::endl;
			}
			/**
			 * @brief Destructor.
			 */
			~JSONStream()
			{
				os << std::endl << "]";
			}
			/**
			 * @brief Add JSON object as array element.
			 *
			 * @param obj JSON object.
			 *
			 * @return JSONStream itself.
			 */
			JSONStream& operator<<(const JSONObject& obj)
			{
				if(flag)
				{
					os << "," << std::endl;
				}
				else
				{
					flag=true;
				}
				os << obj;
				return *this;
			}
	};
}
