#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#ifdef _WIN64
#include <Windows.h>
#include <vcruntime_exception.h>
#else
#include <stdexcept>
#endif

#include <new>
#include <utility>
#include <type_traits>
#include <typeinfo>
#include <string>
#include <cstddef>
#include <initializer_list>

#ifndef ___constexpr20___
	#ifdef _MSVC_LANG
		#if _MSVC_LANG > 201703L
			#define ___constexpr20___ constexpr
		#else
			#define ___constexpr20___
		#endif
	#else
		#if __cplusplus > 201703L
			#define ___constexpr20___ constexpr
		#else
			#define ___constexpr20___
		#endif
	#endif
#endif

#ifndef ___nodiscard___
	#ifdef _MSVC_LANG
		#if _MSVC_LANG > 201402L
			#define ___nodiscard___ [[nodiscard]]
		#else
			#define ___nodiscard___
		#endif
	#else
		#if __cplusplus > 201402L
			#define ___nodiscard___ [[nodiscard]]
		#else
			#define ___nodiscard___
		#endif
	#endif
#endif

namespace dt0
{
	/* Inheritance pair */
	template <typename F, typename S, bool conditions = ((std::is_class_v<F>&& std::is_class_v<S>) &&
							    !(std::is_final_v<F>&& std::is_final_v<S>))>
	class inheritance_pair final : public F, public S
	{
	public:
		using base1 = F;
		using base2 = S;
	};

	/* If conditions not met it's just an empty class */
	template <typename F, typename S>
	class inheritance_pair<F, S, false> final {};

	/* Cluster error */
	class sequence_error : public std::exception
	{
	public:
		using base = std::exception;

		sequence_error() noexcept = default;

		sequence_error(char const* const message) noexcept : base(message) {}

		sequence_error(const std::string& message) noexcept : base(message.c_str()) {}

		sequence_error(std::string&& message) noexcept : base(message.c_str()) {}

		sequence_error(const sequence_error& other) noexcept : base(other) {}

		const sequence_error& operator= (const std::exception& other)
		{
			base::operator= (other);
			return *this;
		}

		const sequence_error& operator= (const sequence_error& other)
		{
			base::operator= (other);
			return *this;
		}

		~sequence_error() noexcept = default;
	};

	/* Cluster unit allocator */
	template <typename U>
	class sequence_allocator
	{
	public:
		/* Heap module data */
		class sequence_heap_module
		{
		public:
			using value_pointer = U*;
			using module_pointer = sequence_heap_module*;

			value_pointer  _begin;
			value_pointer  _end;
			module_pointer _antecessor;
			module_pointer _successor;

			sequence_heap_module() noexcept = default;

			sequence_heap_module(value_pointer const front, value_pointer const back,
				module_pointer const previous, module_pointer const next)
				noexcept : _begin(front), _end(back), _antecessor(previous), _successor(next)
			{}

			~sequence_heap_module() noexcept = default;
		};

		using value_type = U;
		using module_type = sequence_heap_module;
		using const_lvalue_reference = const U&;
		using lvalue_reference = U&;
		using rvalue_reference = U&&;
		using value_pointer = U*;
		using module_pointer = sequence_heap_module*;
		using generic_pointer = void*;
		using byte_pointer = unsigned char*;
		using size_type = std::size_t;

	#ifdef _WIN64
	private:
		/* Core heap */
		generic_pointer _heap = nullptr;

	public:
	#endif
		___constexpr20___ sequence_allocator() noexcept = default;

		___constexpr20___ ~sequence_allocator() noexcept = default;

	#ifdef _WIN64
		___constexpr20___ void set_heap()
		{
			_heap = GetProcessHeap();

			if (_heap == nullptr)
			{
				throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
					std::string(">->set_heap()->WINAPI:\nGetProcessHeap() failed to retrieve default heap of the calling process!"));
			}
		}

		___constexpr20___ void set_heap(const size_type initial_size, const size_type maximum_size)
		{
			_heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, initial_size, maximum_size);

			if (_heap == nullptr)
			{
				throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
					std::string(">->set_heap(const size_type, const size_type)->WINAPI:\nHeapCreate(DWORD, SIZE_T, SIZE_T) failed to create private heap!"));
			}

			auto _result = HeapLock(_heap);

			if (_result != TRUE)
			{
				throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
					std::string(">->set_heap(const size_type, const size_type)->WINAPI:\nHeapLock(HANDLE) failed to lock private heap!"));
			}
		}

		___constexpr20___ void free_heap()
		{
			if (_heap == nullptr)
			{
				throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
					std::string(">->free_heap(): Heap pointer is null!"));
			}

			if (GetProcessHeap() == nullptr)
			{
				throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
					std::string(">->free_heap()->WINAPI:\nGetProcessHeap() failed to retrieve default heap of the calling process!"));
			}

			if (_heap != GetProcessHeap())
			{
				auto _result = HeapUnlock(_heap);

				if (_result != TRUE)
				{
					throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
						std::string(">->free_heap()->WINAPI:\nHeapUnlock(HANDLE) failed to unlock private heap!"));
				}

				_result = HeapDestroy(_heap);

				if (_result != TRUE)
				{
					throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
						std::string(">->free_heap()->WINAPI:\nHeapDestroy(HANDLE) failed to destroy private heap!"));
				}

				_heap = nullptr;
			}
		}
	#endif

		___nodiscard___ ___constexpr20___ module_pointer new_module(const size_type array_size, module_pointer const previous, module_pointer const next)
		{
		#ifdef _WIN64
			if (_heap != nullptr)
			{
				byte_pointer _new_chunk = reinterpret_cast<byte_pointer>(HeapAlloc(_heap, 0, (sizeof(module_type) + (array_size * sizeof(value_type)))));

				if (_new_chunk == nullptr)
				{
					throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
						std::string(">->\nnew_module(const size_type, module_pointer const, module_pointer const)->WINAPI:\nHeapAlloc(HANDLE, DWORD, SIZE_T) failed to allocate new chunk!"));
				}

				module_pointer _output = reinterpret_cast<module_pointer>(_new_chunk);

				::new(_output) module_type(reinterpret_cast<value_pointer>((_new_chunk + sizeof(module_type))),
					reinterpret_cast<value_pointer>((_new_chunk + (sizeof(module_type) + (array_size * sizeof(value_type))))),
					previous, next);

				return _output;
			}

			else
			{
				throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
					std::string(">->\nnew_module(const size_type, module_pointer const, module_pointer const):\nHeap pointer is null!"));
			}

		#else
			try
			{
				byte_pointer _new_chunk = reinterpret_cast<byte_pointer>(::operator new((sizeof(module_type) + (array_size * sizeof(value_type)))));
			}

			catch (std::exception _error)
			{
				throw cluster_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
					std::string(">\nnew_module(const size_type, module_pointer const, module_pointer const)->\n::operator new(size_type): ") +
					std::string(_error.what()));
			}

			module_pointer _output = reinterpret_cast<module_pointer>(_new_chunk);

			::new(_output) module_type(reinterpret_cast<value_pointer>((_new_chunk + sizeof(module_type))),
				reinterpret_cast<value_pointer>((_new_chunk + (sizeof(module_type) + (array_size * sizeof(value_type))))),
				previous, next);

			return _output;
		#endif
		}

		___constexpr20___ void free_module(module_pointer current)
		{
		#ifdef _WIN64
			if (_heap != nullptr)
			{
				current->~module_type();

				auto _result = HeapFree(_heap, 0, reinterpret_cast<generic_pointer>(current));

				if (_result != TRUE)
				{
					throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
						std::string(">->free_module(module_pointer)->WINAPI:\nHeapFree(HANDLE, DWORD, LPVOID) failed to deallocate heap module!"));
				}
			}

			else
			{
				throw sequence_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
					std::string(">->free_module(module_pointer):\nHeap pointer is null!"));
			}

		#else
			try
			{
				::operator delete(reinterpret_cast<generic_pointer>(current), ((static_cast<size_type>(_current->_end - _current->_begin) * sizeof(value_type)) + sizeof(module_type)))
			}

			catch (std::exception _error)
			{
				throw cluster_error(std::string("sequence_allocator<") + std::string(typeid(value_type).name()) +
					std::string(">->free_module(module_pointer)->\n::operator delete(void*, size_type): ") + std::string(_error.what()));
			}
		#endif
		}

	#ifdef _WIN64
		___nodiscard___ bool heap_pointer_null() const
		{
			if (_heap == nullptr)
				return true;

			return false;
		}
	#endif
	};

	template <typename U, typename M>
	class sequence_iterator
	{
	public:
		using value_type = U;
		using module_type = M;
		using const_lvalue_reference = const U&;
		using lvalue_reference = U&;
		using rvalue_reference = U&&;
		using value_pointer = U*;
		using module_pointer = M*;
		using size_type = std::size_t;

	private:
		struct iterator_core
		{
			value_pointer _values_iterator{};
			module_pointer _modules_iterator{};
		};

		iterator_core _core{ nullptr, nullptr };

	public:
		___constexpr20___ sequence_iterator() noexcept = default;

		___constexpr20___ sequence_iterator(std::nullptr_t) noexcept
		{
			_core._modules_iterator = nullptr;
			_core._values_iterator = nullptr;
		}

		___constexpr20___ sequence_iterator(module_pointer const current_module, value_pointer const current_value) noexcept
		{
			_core._modules_iterator = current_module;
			_core._values_iterator = current_value;
		}

		___constexpr20___ sequence_iterator(const sequence_iterator& other) noexcept
		{
			_core._modules_iterator = other.current_module();
			_core._values_iterator = other.current_value();
		}

		___constexpr20___ sequence_iterator(sequence_iterator&& other) noexcept
		{
			_core._modules_iterator = other.current_module();
			_core._values_iterator = other.current_value();
		}

		___constexpr20___ ~sequence_iterator() noexcept = default;

		___nodiscard___ ___constexpr20___ module_pointer const current_module() const
		{
			return _core._modules_iterator;
		}

		___nodiscard___ ___constexpr20___ value_pointer const current_value() const
		{
			return _core._values_iterator;
		}

		___nodiscard___ ___constexpr20___ module_pointer const previous_module() const
		{
			return _core._modules_iterator->_antecessor;
		}

		___nodiscard___ ___constexpr20___ module_pointer const next_module() const
		{
			return _core._modules_iterator->_successor;
		}

		___nodiscard___ ___constexpr20___ value_pointer const array_begin() const
		{
			return _core._modules_iterator->_begin;
		}

		___nodiscard___ ___constexpr20___ value_pointer const array_end() const
		{
			return _core._modules_iterator->_end;
		}

		___nodiscard___ ___constexpr20___ value_pointer const array_rbegin() const
		{
			return (_core._modules_iterator->_end - 1);
		}

		___nodiscard___ ___constexpr20___ value_pointer const array_rend() const
		{
			return (_core._modules_iterator->_begin - 1);
		}

		___constexpr20___ const sequence_iterator& operator= (std::nullptr_t) noexcept
		{
			_core._modules_iterator = nullptr;
			_core._values_iterator = nullptr;

			return *this;
		}

		___constexpr20___ const sequence_iterator& operator= (const sequence_iterator& other) noexcept
		{
			_core._modules_iterator = other.current_module();
			_core._values_iterator = other.current_value();

			return *this;
		}

		___constexpr20___ const sequence_iterator& operator= (sequence_iterator&& other) noexcept
		{
			_core._modules_iterator = other.current_module();
			_core._values_iterator = other.current_value();

			return *this;
		}

		value_pointer const operator-> () const
		{
			return _core._values_iterator;
		}

		___nodiscard___ ___constexpr20___ const_lvalue_reference operator* () const
		{
			return *_core._values_iterator;
		}

		___nodiscard___ ___constexpr20___ lvalue_reference operator* ()
		{
			return *_core._values_iterator;
		}

		___constexpr20___ const sequence_iterator& operator++ ()
		{
			++_core._values_iterator;

			if (_core._values_iterator == _core._modules_iterator->_end)
			{
				if (_core._modules_iterator->_successor != nullptr)
				{
					_core._modules_iterator = _core._modules_iterator->_successor;
					_core._values_iterator = _core._modules_iterator->_begin;
				}
			}

			return *this;
		}

		___constexpr20___ const sequence_iterator& operator-- ()
		{
			--_core._values_iterator;

			if (_core._values_iterator == (_core._modules_iterator->_begin - 1))
			{
				if (_core._modules_iterator->_antecessor != nullptr)
				{
					_core._modules_iterator = _core._modules_iterator->_antecessor;
					_core._values_iterator = (_core._modules_iterator->_end - 1);
				}
			}

			return *this;
		}

		___constexpr20___ sequence_iterator operator++ (int)
		{
			sequence_iterator _output = *this;

			++(*this);

			return _output;
		}

		___constexpr20___ sequence_iterator operator-- (int)
		{
			sequence_iterator _output = *this;

			--(*this);

			return _output;
		}

		___nodiscard___ ___constexpr20___ sequence_iterator operator+ (const size_type offset) const
		{
			sequence_iterator _output = *this;

			for (size_type i = 0; i < offset; ++i)
				++_output;

			return _output;
		}

		___nodiscard___ ___constexpr20___ sequence_iterator operator- (const size_type offset) const
		{
			sequence_iterator _output = *this;

			for (size_type i = 0; i < offset; ++i)
				--_output;

			return _output;
		}

		___constexpr20___ const sequence_iterator& operator+= (const size_type offset)
		{
			for (size_type i = 0; i < offset; ++i)
				++(*this);

			return *this;
		}

		___constexpr20___ const sequence_iterator& operator-= (const size_type offset)
		{
			for (size_type i = 0; i < offset; ++i)
				--(*this);

			return *this;
		}

		___nodiscard___ ___constexpr20___ bool operator!= (const sequence_iterator& other) const
		{
			if (_core._values_iterator != other.current_value())
				return true;

			return false;
		}

		___nodiscard___ ___constexpr20___ bool operator== (const sequence_iterator& other) const
		{
			if (_core._values_iterator == other.current_value())
				return true;

			return false;
		}
	};

	/* Double-ended container that is a sequence of doubly-linked heap modules 
	which each have a reserved space for the user data and the heap module manager class */
	template <typename U, typename A = sequence_allocator<U>>
	class sequence
	{
	public:
		using allocator_type = A;

		using value_type = typename allocator_type::value_type;
		using module_type = typename allocator_type::module_type;
		using const_lvalue_reference = typename allocator_type::const_lvalue_reference;
		using lvalue_reference = typename allocator_type::lvalue_reference;
		using rvalue_reference = typename allocator_type::rvalue_reference;
		using value_pointer = typename allocator_type::value_pointer;
		using module_pointer = typename allocator_type::module_pointer;
		using generic_pointer = typename allocator_type::generic_pointer;
		using byte_pointer = typename allocator_type::byte_pointer;
		using size_type = typename allocator_type::size_type;

		using iterator = sequence_iterator<value_type, module_type>;	// Bidirectional, ++ for forward, -- for reverse iteration

	public:
		/* Default constructor */
		___constexpr20___ sequence() noexcept(false)
		{
		#ifdef _WIN64
			try
			{
				_core.set_heap();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		#endif
		}

		/* Create private heap with specified sizes in bytes, uses HeapCreate(DWORD, SIZE_T, SIZE_T) internally */
		___constexpr20___ sequence(const size_type initial_size, const size_type maximum_size)
		{
		#ifdef _WIN64
			try
			{
				_core.set_heap(initial_size, maximum_size);
			}

			catch (sequence_error _error)
			{
				throw;
			}
		#endif
		}

		/* Initializer list assignment constructor */
		___constexpr20___ sequence(std::initializer_list<value_type> initializer_array)
		{
		#ifdef _WIN64
			try
			{
				_core.set_heap();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		#endif

			try
			{
				for (auto _iterator = initializer_array.begin(); _iterator != initializer_array.end(); ++_iterator)
					this->_write_to_back(*_iterator);
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		/* Initializer list assignment */
		___constexpr20___ const sequence& operator= (std::initializer_list<value_type> initializer_array)
		{
			if (!this->empty())
			{
				try
				{
					this->_free_cluster_unit();
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}

		#ifdef _WIN64
			if (_core.heap_pointer_null())
			{
				try
				{
					_core.set_heap();
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}
		#endif

			try
			{
				for (auto _iterator = initializer_array.begin(); _iterator != initializer_array.end(); ++_iterator)
					this->_write_to_back(*_iterator);
			}

			catch (sequence_error _error)
			{
				throw;
			}

			return *this;
		}

		/* Copy assignment */
		___constexpr20___ sequence(const sequence& other)
		{
			if (this == __builtin_addressof(other))
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">: Self-assignment not allowed"));
			}

		#ifdef _WIN64
			try
			{
				_core.set_heap();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		#endif

			try
			{
				for (iterator _iterator = other.begin(); _iterator != other.end(); ++_iterator)
					this->_write_to_back<const_lvalue_reference>(*_iterator);
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		/* Move constructor */
		___constexpr20___ sequence(sequence&& other) noexcept(false)
		{
			if (this == __builtin_addressof(other))
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">: Self-assignment not allowed"));
			}

		#ifdef _WIN64
			try
			{
				_core.set_heap();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		#endif

			try
			{
				for (iterator _iterator = other.begin(); _iterator != other.end(); ++_iterator)
					this->_write_to_back<rvalue_reference>(std::move(*_iterator));
			}

			catch (sequence_error _error)
			{
				throw;
			}

			other.clear();
		}

		/* Range assignment constructor */
		template <typename range_type>
		___constexpr20___ sequence(range_type other)
		{
			if (reinterpret_cast<generic_pointer>(this) == reinterpret_cast<generic_pointer>(__builtin_addressof(other)))
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">: Self-assignment not allowed"));
			}

		#ifdef _WIN64
			try
			{
				_core.set_heap();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		#endif

			try
			{
				for (auto _iterator = other.begin(); _iterator != other.end(); ++_iterator)
					this->_write_to_back<rvalue_reference>(std::move(*_iterator));
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		/* Assign cluster unit using cluster unit iterators (written specifcally for the cluster get function) */
		___constexpr20___ sequence(iterator beginning, iterator post_end)
		{
			if ((this->begin() == beginning) && (this->end() == post_end))
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">: Self-assignment not allowed!"));
			}

		#ifdef _WIN64
			try
			{
				_core.set_heap();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		#endif

			try
			{
				for (iterator _iterator = beginning; _iterator != post_end; ++_iterator)
					this->_write_to_back<const_lvalue_reference>(*_iterator);
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		/* Assign cluster unit using cluster unit iterators */
		template <typename other_iterator>
		___constexpr20___ sequence(other_iterator beginning, other_iterator post_end)
		{
			if ((this->begin() == beginning) && (this->end() == post_end))
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">: Self-assignment not allowed!"));
			}

		#ifdef _WIN64
			try
			{
				_core.set_heap();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		#endif

			try
			{
				for (other_iterator _iterator = beginning; _iterator != post_end; ++_iterator)
					this->_write_to_back<const_lvalue_reference>(*_iterator);
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		/* Constructor which sets heap reserve space capacity */
		___constexpr20___ sequence(const size_type reserve_capacity)
		{
			_heap_module_reserve_capacity = reserve_capacity;

		#ifdef _WIN64
			try
			{
				_core.set_heap();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		#endif
		}

		/* Copy assignment */
		___constexpr20___ const sequence& operator= (const sequence& other)
		{
			if (this == __builtin_addressof(other))
				return *this;

			if (!this->empty())
			{
				try
				{
					this->_free_cluster_unit();
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}

		#ifdef _WIN64
			if (_core.heap_pointer_null())
			{
				try
				{
					_core.set_heap();
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}
		#endif

			try
			{
				for (auto _iterator = other.begin(); _iterator != other.end(); ++_iterator)
					this->_write_to_back<const_lvalue_reference>(*_iterator);
			}

			catch (sequence_error _error)
			{
				throw;
			}

			return *this;
		}

		/* Move assignment */
		___constexpr20___ const sequence& operator= (sequence&& other) noexcept(false)
		{
			if (this == __builtin_addressof(other))
				return *this;

			if (!this->empty())
			{
				try
				{
					this->_free_cluster_unit();
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}

		#ifdef _WIN64
			if (_core.heap_pointer_null())
			{
				try
				{
					_core.set_heap();
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}
		#endif

			try
			{
				for (auto _iterator = other.begin(); _iterator != other.end(); ++_iterator)
					this->_write_to_back<rvalue_reference>(std::move(*_iterator));
			}
				
			catch (sequence_error _error)
			{
				throw;
			}

			other.clear();

			return *this;
		}

		/* Range assignment */
		template <typename range_type>
		___constexpr20___ const sequence& operator= (range_type other)
		{
			if (reinterpret_cast<generic_pointer>(this) == reinterpret_cast<generic_pointer>(__builtin_addressof(other)))
				return *this;

			if (!this->empty())
			{
				try
				{
					this->_free_cluster_unit();
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}

		#ifdef _WIN64
			if (_core.heap_pointer_null())
			{
				try
				{
					_core.set_heap();
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}
		#endif

			try
			{
				for (auto _iterator = other.begin(); _iterator != other.end(); ++_iterator)
					this->_write_to_back<rvalue_reference>(std::move(*_iterator));
			}

			catch (sequence_error _error)
			{
				throw;
			}

			return *this;
		}

		/* Destructor */
		___constexpr20___ ~sequence() noexcept(false)
		{
			try
			{
				this->_free_cluster_unit();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

	private:
		/* Internal: Write to front */
		template <typename R> ___constexpr20___ void _write_to_front(R _val)
		{
			if ((_core._front == nullptr) && (_core._back == nullptr))
			{
				try
				{
					_core._front_module = _core.new_module(_heap_module_reserve_capacity, nullptr, nullptr);
				}

				catch (sequence_error _error)
				{
					throw;
				}

				_core._front = (_core._front_module->_end - 1);

				_core._back = _core._front;

				_core._back_module = _core._front_module;

				if (typeid(R) == typeid(const_lvalue_reference))
				{
					if (std::is_copy_constructible_v<value_type>)
						::new(_core._front) value_type(_val);

					else if (std::is_copy_assignable_v<value_type>)
						*_core._front = _val;

					else
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_write_to_front(R): Can not assign value!"));
					}
				}

				else
				{
					if (std::is_move_constructible_v<value_type>)
						::new(_core._front) value_type(std::move(_val));

					else if (std::is_move_assignable_v<value_type>)
						*_core._front = std::move(_val);

					else
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_write_to_front(R): Can not assign value!"));
					}
				}
			}

			else
			{
				if ((_core._front - 1) != (_core._front_module->_begin - 1))
				{
					--_core._front;

					if (typeid(R) == typeid(const_lvalue_reference))
					{
						if (std::is_copy_constructible_v<value_type>)
							::new(_core._front) value_type(_val);

						else if (std::is_copy_assignable_v<value_type>)
							*_core._front = _val;

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_write_to_front(R): Can not assign value!"));
						}
					}

					else
					{
						if (std::is_move_constructible_v<value_type>)
							::new(_core._front) value_type(std::move(_val));

						else if (std::is_move_assignable_v<value_type>)
							*_core._front = std::move(_val);

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_write_to_front(R): Can not assign value!"));
						}
					}
				}

				else
				{
					if (_core._front_module->_antecessor != nullptr)
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_write_to_front(R): Antecessor is either occupied or unusable!"));
					}

					try
					{
						_core._front_module->_antecessor = _core.new_module(_heap_module_reserve_capacity, nullptr, _core._front_module);
					}

					catch (sequence_error _error)
					{
						throw;
					}

					_core._front_module = _core._front_module->_antecessor;

					_core._front = (_core._front_module->_end - 1);

					if (typeid(R) == typeid(const_lvalue_reference))
					{
						if (std::is_copy_constructible_v<value_type>)
							::new(_core._front) value_type(_val);

						else if (std::is_copy_assignable_v<value_type>)
							*_core._front = _val;

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_write_to_front(R): Can not assign value!"));
						}
					}

					else
					{
						if (std::is_move_constructible_v<value_type>)
							::new(_core._front) value_type(std::move(_val));

						else if (std::is_move_assignable_v<value_type>)
							*_core._front = std::move(_val);

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_write_to_front(R): Can not assign value!"));
						}
					}
				}
			}
		}

		/* Internal: Writes to back */
		template <typename R> ___constexpr20___ void _write_to_back(R _val)
		{
			if ((_core._front == nullptr) && (_core._back == nullptr))
			{
				try
				{
					_core._back_module = _core.new_module(_heap_module_reserve_capacity, nullptr, nullptr);
				}

				catch (sequence_error _error)
				{
					throw;
				}

				_core._back = _core._back_module->_begin;

				_core._front = _core._back;

				_core._front_module = _core._back_module;

				if (typeid(R) == typeid(const_lvalue_reference))
				{
					if (std::is_copy_constructible_v<value_type>)
						::new(_core._back) value_type(_val);

					else if (std::is_copy_assignable_v<value_type>)
						*_core._back = _val;

					else
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_write_to_back(R): Can not assign value!"));
					}
				}

				else
				{
					if (std::is_move_constructible_v<value_type>)
						::new(_core._back) value_type(std::move(_val));

					else if (std::is_move_assignable_v<value_type>)
						*_core._back = std::move(_val);

					else
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_write_to_back(R): Can not assign value!"));
					}
				}
			}

			else
			{
				if ((_core._back + 1) != _core._back_module->_end)
				{
					++_core._back;

					if (typeid(R) == typeid(const_lvalue_reference))
					{
						if (std::is_copy_constructible_v<value_type>)
							::new(_core._back) value_type(_val);

						else if (std::is_copy_assignable_v<value_type>)
							*_core._back = _val;

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_write_to_back(R): Can not assign value!"));
						}
					}

					else
					{
						if (std::is_move_constructible_v<value_type>)
							::new(_core._back) value_type(std::move(_val));

						else if (std::is_move_assignable_v<value_type>)
							*_core._back = std::move(_val);

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_write_to_back(R): Can not assign value!"));
						}
					}
				}

				else
				{
					if (_core._back_module->_successor != nullptr)
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_write_to_back(R): Successor is either occupied or unusable!"));
					}

					try
					{
						_core._back_module->_successor = _core.new_module(_heap_module_reserve_capacity, _core._back_module, nullptr);
					}

					catch (sequence_error _error)
					{
						throw;
					}

					_core._back_module = _core._back_module->_successor;

					_core._back = _core._back_module->_begin;

					if (typeid(R) == typeid(const_lvalue_reference))
					{
						if (std::is_copy_constructible_v<value_type>)
							::new(_core._back) value_type(_val);

						else if (std::is_copy_assignable_v<value_type>)
							*_core._back = _val;

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_write_to_back(R): Can not assign value!"));
						}
					}

					else
					{
						if (std::is_move_constructible_v<value_type>)
							::new(_core._back) value_type(std::move(_val));

						else if (std::is_move_assignable_v<value_type>)
							*_core._back = std::move(_val);

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_write_to_back(R): Can not assign value!"));
						}
					}
				}
			}
		}

		/* Internal: Free from front */
		___constexpr20___ void _free_from_front()
		{
			if (_core._front == _core._back)
			{
				if (std::is_constructible_v<value_type>)
					_core._front->~value_type();

				_core._front = nullptr;
				_core._back = nullptr;

				try
				{
					_core.free_module(_core._front_module);
				}

				catch (sequence_error _error)
				{
					throw;
				}

				_core._front_module = nullptr;
				_core._back_module = nullptr;
			}

			else if ((_core._front + 1) != _core._front_module->_end)
			{
				if (std::is_constructible_v<value_type>)
					_core._front->~value_type();

				++_core._front;
			}

			else
			{
				if (_core._front_module->_successor != nullptr)
				{
					if (std::is_constructible_v<value_type>)
						_core._front->~value_type();

					_core._front_module = _core._front_module->_successor;

					_core._front = _core._front_module->_begin;

					try
					{
						_core.free_module(_core._front_module->_antecessor);
					}

					catch (sequence_error _error)
					{
						throw;
					}

					_core._front_module->_antecessor = nullptr;
				}
			}
		}

		/* Internal: Free from back */
		___constexpr20___ void _free_from_back()
		{
			if (_core._front == _core._back)
			{
				if (std::is_constructible_v<value_type>)
					_core._back->~value_type();

				_core._front = nullptr;
				_core._back = nullptr;

				try
				{
					_core.free_module(_core._back_module);
				}

				catch (sequence_error _error)
				{
					throw;
				}

				_core._front_module = nullptr;
				_core._back_module = nullptr;
			}

			else if ((_core._back - 1) != (_core._back_module->_begin - 1))
			{
				if (std::is_constructible_v<value_type>)
					_core._back->~value_type();

				--_core._back;
			}

			else
			{
				if (_core._back_module->_antecessor != nullptr)
				{
					if (std::is_constructible_v<value_type>)
						_core._back->~value_type();

					_core._back_module = _core._back_module->_antecessor;

					_core._back = (_core._back_module->_end - 1);

					try
					{
						_core.free_module(_core._back_module->_successor);
					}

					catch (sequence_error _error)
					{
						throw;
					}

					_core._back_module->_successor = nullptr;
				}
			}
		}

		/* Internal: Frees cluster unit */
		___constexpr20___ void _free_cluster_unit()
		{
			if ((_core._front != nullptr) && (_core._back != nullptr))
			{
				while (_core._front != _core._back)
				{
					try
					{
						this->_free_from_back();
					}

					catch (sequence_error _error)
					{
						throw;
					}
				}

				if (std::is_constructible_v<value_type>)
					_core._front->~value_type();

				_core._front = nullptr;
				_core._back = nullptr;

				try
				{
					_core.free_module(_core._front_module);
				}

				catch (sequence_error _error)
				{
					throw;
				}

				_core._front_module = nullptr;
				_core._back_module = nullptr;

			#ifdef _WIN64
				try
				{
					_core.free_heap();
				}

				catch (sequence_error _error)
				{
					throw;
				}
			#endif
			}
		}

		/* Internal: Inserts value at specified position */
		template <typename R> ___constexpr20___ iterator _insert(iterator&& _left, iterator&& _right, R _val) noexcept(false)
		{
			if (_left.current_module() == _right.current_module())
			{
				size_type _left_distance = static_cast<size_type>(_left.current_value() - _left.array_begin());
				size_type _right_distance = static_cast<size_type>(_right.current_value() - _right.array_begin());

				module_pointer _current_module = _left.current_module();

				module_pointer _previous = _left.previous_module();
				module_pointer _next = _left.next_module();

				if ((_previous != nullptr) && (_next != nullptr))
				{
					size_type _size = static_cast<size_type>(_left.array_end() - _left.array_begin());

					value_pointer _temp_array = new value_type[_size];

					if (std::is_move_assignable_v<value_type>)
					{
						for (value_pointer _iterator = _left.array_begin(); _iterator != _left.array_end(); ++_iterator)
							*(_temp_array + static_cast<size_type>(_iterator - _left.array_begin())) = std::move(*_iterator);
					}

					else if (std::is_copy_assignable_v<value_type>)
					{
						for (value_pointer _iterator = _left.array_begin(); _iterator != _left.array_end(); ++_iterator)
							*(_temp_array + static_cast<size_type>(_iterator - _left.array_begin())) = *_iterator;
					}

					else
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
					}

					if (std::is_constructible_v<value_type>)
					{
						for (value_pointer _iterator = _current_module->_begin; _iterator != _current_module->_end; ++_iterator)
						{
							_iterator->~value_type();
						}
					}

					try
					{
						_core.free_module(_current_module);
					}

					catch (sequence_error _error)
					{
						throw;
					}

					module_pointer _new_module = nullptr;

					try
					{
						_new_module = _core.new_module((_size + 1), _previous, _next);
					}

					catch (sequence_error _error)
					{
						throw;
					}

					_previous->_successor = _new_module;
					_next->_antecessor = _new_module;

					if (typeid(R) == typeid(const_lvalue_reference))
					{
						if (std::is_copy_constructible_v<value_type>)
							::new((_new_module->_begin + _right_distance)) value_type(_val);

						else if (std::is_copy_assignable_v<value_type>)
							*(_new_module->_begin + _right_distance) = _val;

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
						}
					}

					else
					{
						if (std::is_move_constructible_v<value_type>)
							::new((_new_module->_begin + _right_distance)) value_type(std::move(_val));

						else if (std::is_move_assignable_v<value_type>)
							*(_new_module->_begin + _right_distance) = std::move(_val);

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
						}
					}

					if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
					{
						if (std::is_move_constructible_v<value_type>)
						{
							for (size_type i = 0; i < _right_distance; ++i)
							{
								::new((_new_module->_begin + i)) value_type(std::move(_temp_array[i]));
							}

							for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
							{
								::new((_new_module->_begin + i)) value_type(std::move(_temp_array[(i - 1)]));
							}
						}

						else
						{
							for (size_type i = 0; i < _right_distance; ++i)
							{
								*(_new_module->_begin + i) = std::move(_temp_array[i]);
							}

							for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
							{
								*(_new_module->_begin + i) = std::move(_temp_array[(i - 1)]);
							}
						}
					}

					else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
					{
						if (std::is_copy_constructible_v<value_type>)
						{
							for (size_type i = 0; i < _right_distance; ++i)
							{
								::new((_new_module->_begin + i)) value_type(_temp_array[i]);
							}

							for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
							{
								::new((_new_module->_begin + i)) value_type(_temp_array[(i - 1)]);
							}
						}

						else
						{
							for (size_type i = 0; i < _right_distance; ++i)
							{
								*(_new_module->_begin + i) = _temp_array[i];
							}

							for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
							{
								*(_new_module->_begin + i) = _temp_array[(i - 1)];
							}
						}
					}

					else
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
					}

					delete[] _temp_array;

					return iterator(_new_module, (_new_module->_begin + _right_distance));
				}

				else if ((_previous == nullptr) && (_next != nullptr))
				{
					if (_core._front != _core._front_module->_begin)
					{
						--_core._front;

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								::new(_core._front) value_type(std::move(*(_core._front + 1)));
							}

							else
							{
								*_core._front = std::move(*(_core._front + 1));
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								::new(_core._front) value_type(*(_core._front + 1));
							}

							else
							{
								*_core._front = *(_core._front + 1);
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}


						if (std::is_move_assignable_v<value_type>)
						{
							for (value_pointer _iterator = (_core._front + 1); _iterator != (_core._front_module->_begin + _right_distance); ++_iterator)
							{
								*_iterator = std::move(*(_iterator + 1));
							}
						}

						else if (std::is_copy_assignable_v<value_type>)
						{
							for (value_pointer _iterator = (_core._front + 1); _iterator != (_core._front_module->_begin + _right_distance); ++_iterator)
							{
								*_iterator = *(_iterator + 1);
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}

						if (typeid(R) == typeid(const_lvalue_reference))
						{
							if (std::is_copy_constructible_v<value_type>)
								::new((_core._front_module->_begin + _left_distance)) value_type(_val);

							else if (std::is_copy_assignable_v<value_type>)
								*(_core._front_module->_begin + _left_distance) = _val;

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						else
						{
							if (std::is_move_constructible_v<value_type>)
								::new((_core._front_module->_begin + _left_distance)) value_type(std::move(_val));

							else if (std::is_move_assignable_v<value_type>)
								*(_core._front_module->_begin + _left_distance) = std::move(_val);

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						return iterator(_core._front_module, (_core._front_module->_begin + _left_distance));
					}

					else
					{
						size_type _size = static_cast<size_type>(_left.array_end() - _left.array_begin());

						value_pointer _temp_array = new value_type[_size];

						if (std::is_move_assignable_v<value_type>)
						{
							for (value_pointer _iterator = _left.array_begin(); _iterator != _left.array_end(); ++_iterator)
								*(_temp_array + static_cast<size_type>(_iterator - _left.array_begin())) = std::move(*_iterator);
						}

						else if (std::is_copy_assignable_v<value_type>)
						{
							for (value_pointer _iterator = _left.array_begin(); _iterator != _left.array_end(); ++_iterator)
								*(_temp_array + static_cast<size_type>(_iterator - _left.array_begin())) = *_iterator;
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}

						if (std::is_constructible_v<value_type>)
						{
							for (value_pointer _iterator = _current_module->_begin; _iterator != _current_module->_end; ++_iterator)
							{
								_iterator->~value_type();
							}
						}

						try
						{
							_core.free_module(_current_module);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						module_pointer _new_module = nullptr;

						try
						{
							_new_module = _core.new_module((_size + 1), _previous, _next);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						_next->_antecessor = _new_module;

						if (typeid(R) == typeid(const_lvalue_reference))
						{
							if (std::is_copy_constructible_v<value_type>)
								::new((_new_module->_begin + _right_distance)) value_type(_val);

							else if (std::is_copy_assignable_v<value_type>)
								*(_new_module->_begin + _right_distance) = _val;

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						else
						{
							if (std::is_move_constructible_v<value_type>)
								::new((_new_module->_begin + _right_distance)) value_type(std::move(_val));

							else if (std::is_move_assignable_v<value_type>)
								*(_new_module->_begin + _right_distance) = std::move(_val);

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									::new((_new_module->_begin + i)) value_type(std::move(_temp_array[i]));
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									::new((_new_module->_begin + i)) value_type(std::move(_temp_array[(i - 1)]));
								}
							}

							else
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									*(_new_module->_begin + i) = std::move(_temp_array[i]);
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									*(_new_module->_begin + i) = std::move(_temp_array[(i - 1)]);
								}
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_copy_constructible_v<value_type>)
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									::new((_new_module->_begin + i)) value_type(_temp_array[i]);
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									::new((_new_module->_begin + i)) value_type(_temp_array[(i - 1)]);
								}
							}

							else
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									*(_new_module->_begin + i) = _temp_array[i];
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									*(_new_module->_begin + i) = _temp_array[(i - 1)];
								}
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}

						delete[] _temp_array;

						_core._front_module = _new_module;
						_core._front = _core._front_module->_begin;

						return iterator(_new_module, (_new_module->_begin + _right_distance));
					}
				}

				else if ((_previous != nullptr) && (_next == nullptr))
				{
					if (_core._back != (_core._back_module->_end - 1))
					{
						++_core._back;

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								::new(_core._back) value_type(std::move(*(_core._back - 1)));
							}

							else
							{
								*_core._back = std::move(*(_core._back - 1));
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								::new(_core._back) value_type(*(_core._back - 1));
							}

							else
							{
								*_core._back = *(_core._back - 1);
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}


						if (std::is_move_assignable_v<value_type>)
						{
							for (value_pointer _iterator = (_core._back - 1); _iterator != (_core._back_module->_begin + _right_distance); --_iterator)
							{
								*_iterator = std::move(*(_iterator - 1));
							}
						}

						else if (std::is_copy_assignable_v<value_type>)
						{
							for (value_pointer _iterator = (_core._back - 1); _iterator != (_core._back_module->_begin + _right_distance); --_iterator)
							{
								*_iterator = *(_iterator - 1);
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}

						if (typeid(R) == typeid(const_lvalue_reference))
						{
							if (std::is_copy_constructible_v<value_type>)
								::new((_core._back_module->_begin + _right_distance)) value_type(_val);

							else if (std::is_copy_assignable_v<value_type>)
								*(_core._back_module->_begin + _right_distance) = _val;

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						else
						{
							if (std::is_move_constructible_v<value_type>)
								::new((_core._back_module->_begin + _right_distance)) value_type(std::move(_val));

							else if (std::is_move_assignable_v<value_type>)
								*(_core._back_module->_begin + _right_distance) = std::move(_val);

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						return iterator(_core._back_module, (_core._back_module->_begin + _right_distance));
					}

					else
					{
						size_type _size = static_cast<size_type>(_left.array_end() - _left.array_begin());

						value_pointer _temp_array = new value_type[_size];

						if (std::is_move_assignable_v<value_type>)
						{
							for (value_pointer _iterator = _left.array_begin(); _iterator != _left.array_end(); ++_iterator)
								*(_temp_array + static_cast<size_type>(_iterator - _left.array_begin())) = std::move(*_iterator);
						}

						else if (std::is_copy_assignable_v<value_type>)
						{
							for (value_pointer _iterator = _left.array_begin(); _iterator != _left.array_end(); ++_iterator)
								*(_temp_array + static_cast<size_type>(_iterator - _left.array_begin())) = *_iterator;
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}

						if (std::is_constructible_v<value_type>)
						{
							for (value_pointer _iterator = _current_module->_begin; _iterator != _current_module->_end; ++_iterator)
							{
								_iterator->~value_type();
							}
						}

						try
						{
							_core.free_module(_current_module);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						module_pointer _new_module = nullptr;

						try
						{
							_new_module = _core.new_module((_size + 1), _previous, _next);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						_previous->_successor = _new_module;

						if (typeid(R) == typeid(const_lvalue_reference))
						{
							if (std::is_copy_constructible_v<value_type>)
								::new((_new_module->_begin + _right_distance)) value_type(_val);

							else if (std::is_copy_assignable_v<value_type>)
								*(_new_module->_begin + _right_distance) = _val;

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						else
						{
							if (std::is_move_constructible_v<value_type>)
								::new((_new_module->_begin + _right_distance)) value_type(std::move(_val));

							else if (std::is_move_assignable_v<value_type>)
								*(_new_module->_begin + _right_distance) = std::move(_val);

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									::new((_new_module->_begin + i)) value_type(std::move(_temp_array[i]));
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									::new((_new_module->_begin + i)) value_type(std::move(_temp_array[(i - 1)]));
								}
							}

							else
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									*(_new_module->_begin + i) = std::move(_temp_array[i]);
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									*(_new_module->_begin + i) = std::move(_temp_array[(i - 1)]);
								}
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_copy_constructible_v<value_type>)
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									::new((_new_module->_begin + i)) value_type(_temp_array[i]);
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									::new((_new_module->_begin + i)) value_type(_temp_array[(i - 1)]);
								}
							}

							else
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									*(_new_module->_begin + i) = _temp_array[i];
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									*(_new_module->_begin + i) = _temp_array[(i - 1)];
								}
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}

						_core._back_module = _new_module;
						_core._back = (_core._back_module->_end - 1);

						return iterator(_new_module, (_new_module->_begin + _right_distance));
					}
				}

				else
				{
					if (_core._front != _core._front_module->_begin)
					{
						--_core._front;

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								::new(_core._front) value_type(std::move(*(_core._front + 1)));
							}

							else
							{
								*_core._front = std::move(*(_core._front + 1));
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								::new(_core._front) value_type(*(_core._front + 1));
							}

							else
							{
								*_core._front = *(_core._front + 1);
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}


						if (std::is_move_assignable_v<value_type>)
						{
							for (value_pointer _iterator = (_core._front + 1); _iterator != (_core._front_module->_begin + _right_distance); ++_iterator)
							{
								*_iterator = std::move(*(_iterator + 1));
							}
						}

						else if (std::is_copy_assignable_v<value_type>)
						{
							for (value_pointer _iterator = (_core._front + 1); _iterator != (_core._front_module->_begin + _right_distance); ++_iterator)
							{
								*_iterator = *(_iterator + 1);
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}

						if (typeid(R) == typeid(const_lvalue_reference))
						{
							if (std::is_copy_constructible_v<value_type>)
								::new((_core._front_module->_begin + _left_distance)) value_type(_val);

							else if (std::is_copy_assignable_v<value_type>)
								*(_core._front_module->_begin + _left_distance) = _val;

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						else
						{
							if (std::is_move_constructible_v<value_type>)
								::new((_core._front_module->_begin + _left_distance)) value_type(std::move(_val));

							else if (std::is_move_assignable_v<value_type>)
								*(_core._front_module->_begin + _left_distance) = std::move(_val);

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						return iterator(_core._front_module, (_core._front_module->_begin + _left_distance));
					}

					else if (_core._back != (_core._back_module->_end - 1))
					{
						++_core._back;

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								::new(_core._back) value_type(std::move(*(_core._back - 1)));
							}

							else
							{
								*_core._back = std::move(*(_core._back - 1));
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								::new(_core._back) value_type(*(_core._back - 1));
							}

							else
							{
								*_core._back = *(_core._back - 1);
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}


						if (std::is_move_assignable_v<value_type>)
						{
							for (value_pointer _iterator = (_core._back - 1); _iterator != (_core._back_module->_begin + _right_distance); --_iterator)
							{
								*_iterator = std::move(*(_iterator - 1));
							}
						}

						else if (std::is_copy_assignable_v<value_type>)
						{
							for (value_pointer _iterator = (_core._back - 1); _iterator != (_core._back_module->_begin + _right_distance); --_iterator)
							{
								*_iterator = *(_iterator - 1);
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}

						if (typeid(R) == typeid(const_lvalue_reference))
						{
							if (std::is_copy_constructible_v<value_type>)
								::new((_core._back_module->_begin + _right_distance)) value_type(_val);

							else if (std::is_copy_assignable_v<value_type>)
								*(_core._back_module->_begin + _right_distance) = _val;

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						else
						{
							if (std::is_move_constructible_v<value_type>)
								::new((_core._back_module->_begin + _right_distance)) value_type(std::move(_val));

							else if (std::is_move_assignable_v<value_type>)
								*(_core._back_module->_begin + _right_distance) = std::move(_val);

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						return iterator(_core._back_module, (_core._back_module->_begin + _right_distance));
					}

					else
					{
						size_type _size = static_cast<size_type>(_left.array_end() - _left.array_begin());

						value_pointer _temp_array = new value_type[_size];

						if (std::is_move_assignable_v<value_type>)
						{
							for (value_pointer _iterator = _left.array_begin(); _iterator != _left.array_end(); ++_iterator)
								*(_temp_array + static_cast<size_type>(_iterator - _left.array_begin())) = std::move(*_iterator);
						}

						else if (std::is_copy_assignable_v<value_type>)
						{
							for (value_pointer _iterator = _left.array_begin(); _iterator != _left.array_end(); ++_iterator)
								*(_temp_array + static_cast<size_type>(_iterator - _left.array_begin())) = *_iterator;
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}

						if (std::is_constructible_v<value_type>)
						{
							for (value_pointer _iterator = _current_module->_begin; _iterator != _current_module->_end; ++_iterator)
							{
								_iterator->~value_type();
							}
						}

						try
						{
							_core.free_module(_current_module);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						module_pointer _new_module = nullptr;

						try
						{
							_new_module = _core.new_module((_size + 1), _previous, _next);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						if (typeid(R) == typeid(const_lvalue_reference))
						{
							if (std::is_copy_constructible_v<value_type>)
								::new((_new_module->_begin + _right_distance)) value_type(_val);

							else if (std::is_copy_assignable_v<value_type>)
								*(_new_module->_begin + _right_distance) = _val;

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						else
						{
							if (std::is_move_constructible_v<value_type>)
								::new((_new_module->_begin + _right_distance)) value_type(std::move(_val));

							else if (std::is_move_assignable_v<value_type>)
								*(_new_module->_begin + _right_distance) = std::move(_val);

							else
							{
								throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
									std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
							}
						}

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									::new((_new_module->_begin + i)) value_type(std::move(_temp_array[i]));
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									::new((_new_module->_begin + i)) value_type(std::move(_temp_array[(i - 1)]));
								}
							}

							else
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									*(_new_module->_begin + i) = std::move(_temp_array[i]);
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									*(_new_module->_begin + i) = std::move(_temp_array[(i - 1)]);
								}
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_copy_constructible_v<value_type>)
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									::new((_new_module->_begin + i)) value_type(_temp_array[i]);
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									::new((_new_module->_begin + i)) value_type(_temp_array[(i - 1)]);
								}
							}

							else
							{
								for (size_type i = 0; i < _right_distance; ++i)
								{
									*(_new_module->_begin + i) = _temp_array[i];
								}

								for (size_type i = (_right_distance + 1); i < (_size + 1); ++i)
								{
									*(_new_module->_begin + i) = _temp_array[(i - 1)];
								}
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_insert(iterator&&, iterator&&, R): Inner array can neither be moved nor copied to!"));
						}

						_core._front_module = _new_module;
						_core._back_module = _new_module;

						_core._front = _core._front_module->_begin;
						_core._back = (_core._back_module->_end - 1);

						delete[] _temp_array;

						return iterator(_new_module, (_new_module->_begin + _right_distance));
					}
				}
			}

			else
			{
				module_pointer _previous = _left.current_module();
				module_pointer _next = _right.current_module();

				_previous->_successor = nullptr;
				_next->_antecessor = nullptr;

				module_pointer _new_module = nullptr;

				try
				{
					_new_module = _core.new_module(1ui64, _previous, _next);
				}

				catch (sequence_error _error)
				{
					throw;
				}

				_previous->_successor = _new_module;
				_next->_antecessor = _new_module;

				if (typeid(R) == typeid(const_lvalue_reference))
				{
					if (std::is_copy_constructible_v<value_type>)
						::new(_new_module->_begin) value_type(_val);

					else if (std::is_copy_assignable_v<value_type>)
						*_new_module->_begin = _val;

					else
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
					}
				}

				else
				{
					if (std::is_move_constructible_v<value_type>)
						::new(_new_module->_begin) value_type(std::move(_val));

					else if (std::is_move_assignable_v<value_type>)
						*_new_module->_begin = std::move(_val);

					else
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_insert(iterator&&, iterator&&, R): Can not insert value!"));
					}
				}

				return iterator(_new_module, _new_module->_begin);
			}
		}

		___constexpr20___ void _erase(iterator&& _position)
		{
			if (_position.current_module() == _core._front_module)
			{
				if (std::is_move_assignable_v<value_type>)
				{
					for (value_pointer _iterator = _position.current_value(); _iterator != _core._front; --_iterator)
					{
						*_iterator = std::move(*(_iterator - 1));
					}

					_core._front->~value_type();

					++_core._front;
				}

				else if (std::is_copy_assignable_v<value_type>)
				{
					for (value_pointer _iterator = _position.current_value(); _iterator != _core._front; --_iterator)
					{
						*_iterator = *(_iterator - 1);
					}

					_core._front->~value_type();

					++_core._front;
				}

				else
				{
					throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
						std::string(">->_erase(iterator&&): Inner array can neither be moved nor copied!"));
				}
			}

			else if (_position.current_module() == _core._back_module)
			{
				if (std::is_move_assignable_v<value_type>)
				{
					for (value_pointer _iterator = _position.current_value(); _iterator != _core._back; ++_iterator)
					{
						*_iterator = std::move(*(_iterator + 1));
					}

					_core._back->~value_type();

					--_core._back;
				}

				else if (std::is_copy_assignable_v<value_type>)
				{
					for (value_pointer _iterator = _position.current_value(); _iterator != _core._back; ++_iterator)
					{
						*_iterator = *(_iterator + 1);
					}

					_core._back->~value_type();

					--_core._back;
				}

				else
				{
					throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
						std::string(">->_erase(iterator&&): Inner array can neither be moved nor copied!"));
				}
			}

			else
			{
				size_type _size = static_cast<size_type>(_position.array_end() - _position.array_begin());

				module_pointer _current = _position.current_module();

				module_pointer _previous = nullptr;
				module_pointer _next = nullptr;

				if ((_current->_antecessor != nullptr) && (_current->_successor != nullptr))
				{
					_previous = _position.previous_module();
					_next = _position.next_module();
				}

				module_pointer _new_module = nullptr;

				try
				{
					_new_module = _core.new_module((_size - 1), _previous, _next);
				}

				catch (sequence_error _error)
				{
					throw;
				}

				if ((_current->_antecessor != nullptr) && (_current->_successor != nullptr))
				{
					_previous->_successor = _new_module;
					_next->_antecessor = _new_module;

					_current->_antecessor = nullptr;
					_current->_successor = nullptr;
				}

				else
				{
					_core._front_module = _new_module;
					_core._back_module = _new_module;

					_core._front = _new_module->_begin;
					_core._back = (_new_module->_end - 1);
				}

				if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
				{
					value_pointer _new_iterator = _new_module->_begin;

					if (std::is_move_constructible_v<value_type>)
					{
						for (value_pointer _iterator = _current->_begin; _iterator != _current->_end; ++_iterator)
						{
							if (_iterator != _position.current_value())
							{
								::new(_new_iterator) value_type(std::move(*_iterator));

								_iterator->~value_type();

								++_new_iterator;
							}
						}

						_position->~value_type();
					}

					else
					{
						for (value_pointer _iterator = _current->_begin; _iterator != _current->_end; ++_iterator)
						{
							if (_iterator != _position.current_value())
							{
								*_new_iterator = std::move(*_iterator);

								++_new_iterator;
							}
						}
					}
				}

				else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
				{
					value_pointer _new_iterator = _new_module->_begin;

					if (std::is_copy_constructible_v<value_type>)
					{
						for (value_pointer _iterator = _current->_begin; _iterator != _current->_end; ++_iterator)
						{
							if (_iterator != _position.current_value())
							{
								::new(_new_iterator) value_type(*_iterator);

								_iterator->~value_type();

								++_new_iterator;
							}
						}

						_position->~value_type();
					}

					else
					{
						for (value_pointer _iterator = _current->_begin; _iterator != _current->_end; ++_iterator)
						{
							if (_iterator != _position.current_value())
							{
								*_new_iterator = *_iterator;

								++_new_iterator;
							}
						}
					}
				}

				else
				{
					throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
						std::string(">->_erase(iterator&&): Inner array can neither be moved nor copied!"));
				}

				try
				{
					_core.free_module(_current);
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}
		}

		___constexpr20___ void _compress()
		{
			if ((_core._front != _core._front_module->_begin) || (_core._back != (_core._back_module->_end - 1)))
			{
				if (_core._front_module == _core._back_module)
				{
					size_type _size = static_cast<size_type>((_core._back + 1) - _core._front);

					module_pointer _new_module = nullptr;

					try
					{
						_new_module = _core.new_module(_size, nullptr, nullptr);
					}

					catch (sequence_error _error)
					{
						throw;
					}

					if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
					{
						if (std::is_move_constructible_v<value_type>)
						{
							value_pointer _new_front_iterator = _new_module->_begin;

							for (value_pointer _iterator = _core._front; _iterator != (_core._back + 1); ++_iterator)
							{
								::new(_new_front_iterator) value_type(std::move(*_iterator));

								_iterator->~value_type();

								++_new_front_iterator;
							}
						}

						else
						{
							value_pointer _new_front_iterator = _new_module->_begin;

							for (value_pointer _iterator = _core._front; _iterator != (_core._back + 1); ++_iterator)
							{
								*_new_front_iterator = std::move(*_iterator);

								++_new_front_iterator;
							}
						}
					}

					else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
					{
						if (std::is_copy_constructible_v<value_type>)
						{
							value_pointer _new_front_iterator = _new_module->_begin;

							for (value_pointer _iterator = _core._front; _iterator != (_core._back + 1); ++_iterator)
							{
								::new(_new_front_iterator) value_type(*_iterator);

								_iterator->~value_type();

								++_new_front_iterator;
							}
						}

						else
						{
							value_pointer _new_front_iterator = _new_module->_begin;

							for (value_pointer _iterator = _core._front; _iterator != (_core._back + 1); ++_iterator)
							{
								*_new_front_iterator = *_iterator;

								++_new_front_iterator;
							}
						}
					}

					else
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->_compress(): Inner array can neither be moved nor copied!"));
					}

					module_pointer _old_module = _core._front_module;

					_core._front_module = _new_module;
					_core._back_module = _new_module;

					_core._front = _core._front_module->_begin;
					_core._back = (_core._back_module->_end - 1);

					try
					{
						_core.free_module(_old_module);
					}

					catch (sequence_error _error)
					{
						throw;
					}
				}

				else if ((_core._front_module->_successor == _core._back_module) && (_core._back_module->_antecessor == _core._front_module))
				{
					if (_core._back != (_core._back_module->_end - 1))
					{
						module_pointer _new_back = nullptr;

						try
						{
							_new_back = _core.new_module(static_cast<size_type>((_core._back + 1) - _core._back_module->_begin), _core._front_module, nullptr);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						_core._front_module->_successor = _new_back;

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								value_pointer _new_back_iterator = _new_back->_begin;

								for (value_pointer _iterator = _core._back_module->_begin; _iterator != (_core._back + 1); ++_iterator)
								{
									::new(_new_back_iterator) value_type(std::move(*_iterator));

									_iterator->~value_type();

									++_new_back_iterator;
								}
							}

							else
							{
								value_pointer _new_back_iterator = _new_back->_begin;

								for (value_pointer _iterator = _core._back_module->_begin; _iterator != (_core._back + 1); ++_iterator)
								{
									*_new_back_iterator = std::move(*_iterator);

									++_new_back_iterator;
								}
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_copy_constructible_v<value_type>)
							{
								value_pointer _new_back_iterator = _new_back->_begin;

								for (value_pointer _iterator = _core._back_module->_begin; _iterator != (_core._back + 1); ++_iterator)
								{
									::new(_new_back_iterator) value_type(*_iterator);

									_iterator->~value_type();

									++_new_back_iterator;
								}
							}

							else
							{
								value_pointer _new_back_iterator = _new_back->_begin;

								for (value_pointer _iterator = _core._back_module->_begin; _iterator != (_core._back + 1); ++_iterator)
								{
									*_new_back_iterator = *_iterator;

									++_new_back_iterator;
								}
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_compress(): Inner array can neither be moved nor copied!"));
						}

						try
						{
							_core.free_module(_core._back_module);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						_core._back_module = _new_back;
						_core._back = (_core._back_module->_end - 1);
					}

					if (_core._front != _core._front_module->_begin)
					{
						module_pointer _new_front = nullptr;

						try
						{
							_new_front = _core.new_module(static_cast<size_type>(_core._front_module->_end - _core._front), nullptr, _core._back_module);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						_core._back_module->_antecessor = _new_front;

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								value_pointer _new_front_iterator = _new_front->_begin;

								for (value_pointer _iterator = _core._front; _iterator != _core._front_module->_end; ++_iterator)
								{
									::new(_new_front_iterator) value_type(std::move(*_iterator));

									_iterator->~value_type();

									++_new_front_iterator;
								}
							}

							else
							{
								value_pointer _new_front_iterator = _new_front->_begin;

								for (value_pointer _iterator = _core._front; _iterator != _core._front_module->_end; ++_iterator)
								{
									*_new_front_iterator = std::move(*_iterator);

									++_new_front_iterator;
								}
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_copy_constructible_v<value_type>)
							{
								value_pointer _new_front_iterator = _new_front->_begin;

								for (value_pointer _iterator = _core._front; _iterator != _core._front_module->_end; ++_iterator)
								{
									::new(_new_front_iterator) value_type(*_iterator);

									_iterator->~value_type();

									++_new_front_iterator;
								}
							}

							else
							{
								value_pointer _new_front_iterator = _new_front->_begin;

								for (value_pointer _iterator = _core._front; _iterator != _core._front_module->_end; ++_iterator)
								{
									*_new_front_iterator = *_iterator;

									++_new_front_iterator;
								}
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_compress(): Inner array can neither be moved nor copied!"));
						}

						try
						{
							_core.free_module(_core._front_module);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						_core._front_module = _new_front;
						_core._front = _core._front_module->_begin;
					}
				}

				else
				{
					if (_core._front != _core._front_module->_begin)
					{
						module_pointer _front_successor = _core._front_module->_successor;
						module_pointer _new_front = nullptr;

						size_type _front_size = static_cast<size_type>(_core._front_module->_end - _core._front);

						try
						{
							_new_front = _core.new_module(_front_size, nullptr, _front_successor);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						_front_successor->_antecessor = _new_front;

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								value_pointer _new_front_iterator = _new_front->_begin;

								for (value_pointer _iterator = _core._front; _iterator != _core._front_module->_end; ++_iterator)
								{
									::new(_new_front_iterator) value_type(std::move(*_iterator));

									_iterator->~value_type();

									++_new_front_iterator;
								}
							}

							else
							{
								value_pointer _new_front_iterator = _new_front->_begin;

								for (value_pointer _iterator = _core._front; _iterator != _core._front_module->_end; ++_iterator)
								{
									*_new_front_iterator = std::move(*_iterator);

									++_new_front_iterator;
								}
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_copy_constructible_v<value_type>)
							{
								value_pointer _new_front_iterator = _new_front->_begin;

								for (value_pointer _iterator = _core._front; _iterator != _core._front_module->_end; ++_iterator)
								{
									::new(_new_front_iterator) value_type(*_iterator);

									_iterator->~value_type();

									++_new_front_iterator;
								}
							}

							else
							{
								value_pointer _new_front_iterator = _new_front->_begin;

								for (value_pointer _iterator = _core._front; _iterator != _core._front_module->_end; ++_iterator)
								{
									*_new_front_iterator = *_iterator;

									++_new_front_iterator;
								}
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_compress(): Inner array can neither be moved nor copied!"));
						}

						try
						{
							_core.free_module(_core._front_module);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						_core._front_module = _new_front;
						_core._front = _core._front_module->_begin;
					}

					if (_core._back != (_core._back_module->_end - 1))
					{
						module_pointer _back_antecessor = _core._back_module->_antecessor;
						module_pointer _new_back = nullptr;

						size_type _back_size = static_cast<size_type>((_core._back + 1) - _core._back_module->_begin);

						try
						{
							_new_back = _core.new_module(_back_size, _back_antecessor, nullptr);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						_back_antecessor->_successor = _new_back;

						if (std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>)
						{
							if (std::is_move_constructible_v<value_type>)
							{
								value_pointer _new_back_iterator = _new_back->_begin;

								for (value_pointer _iterator = _core._back_module->_begin; _iterator != (_core._back + 1); ++_iterator)
								{
									::new(_new_back_iterator) value_type(std::move(*_iterator));

									_iterator->~value_type();

									++_new_back_iterator;
								}
							}

							else
							{
								value_pointer _new_back_iterator = _new_back->_begin;

								for (value_pointer _iterator = _core._back_module->_begin; _iterator != (_core._back + 1); ++_iterator)
								{
									*_new_back_iterator = std::move(*_iterator);

									++_new_back_iterator;
								}
							}
						}

						else if (std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>)
						{
							if (std::is_copy_constructible_v<value_type>)
							{
								value_pointer _new_back_iterator = _new_back->_begin;

								for (value_pointer _iterator = _core._back_module->_begin; _iterator != (_core._back + 1); ++_iterator)
								{
									::new(_new_back_iterator) value_type(*_iterator);

									_iterator->~value_type();

									++_new_back_iterator;
								}
							}

							else
							{
								value_pointer _new_back_iterator = _new_back->_begin;

								for (value_pointer _iterator = _core._back_module->_begin; _iterator != (_core._back + 1); ++_iterator)
								{
									*_new_back_iterator = *_iterator;

									++_new_back_iterator;
								}
							}
						}

						else
						{
							throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
								std::string(">->_compress(): Inner array can neither be moved nor copied!"));
						}

						try
						{
							_core.free_module(_core._back_module);
						}

						catch (sequence_error _error)
						{
							throw;
						}

						_core._back_module = _new_back;
						_core._back = (_core._back_module->_end - 1);
					}
				}
			}
		}

	public:
		___constexpr20___ void set_reserve_capacity(const size_type new_capacity)
		{
			if ((new_capacity == 0) || (new_capacity > (4096 - sizeof(module_type))))
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">->set_reserve_capacity(const size_type): Invalid heap module data reserve capacity!"));
			}

			_heap_module_reserve_capacity = new_capacity;
		}

		___constexpr20___ void push_front(const_lvalue_reference val)
		{
			try
			{
				this->_write_to_front<const_lvalue_reference>(val);
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		___constexpr20___ void push_front(rvalue_reference val)
		{
			try
			{
				this->_write_to_front<rvalue_reference>(std::move(val));
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		___constexpr20___ void push_back(const_lvalue_reference val)
		{
			try
			{
				this->_write_to_back<const_lvalue_reference>(val);
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		___constexpr20___ void push_back(rvalue_reference val)
		{
			try
			{
				this->_write_to_back<rvalue_reference>(std::move(val));
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		___constexpr20___ iterator insert(iterator left, iterator right, const_lvalue_reference val)
		{
			iterator _find_left = this->find(*left);
			iterator _find_right = this->find(*right);

			if ((_find_left == this->end()) || (_find_right == this->end()))
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">->insert(iterator, iterator, const_lvalue_reference): Invalid iterators provided, no such values exist!"));
			}

			if ((left + 1) != right)
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">->insert(iterator, iterator, const_lvalue_reference): Invalid iterators provided!"));
			}

			try
			{
				iterator _output = nullptr;

				if (((left == this->rend()) || (left == this->begin())) && ((right == this->begin()) || (right == this->rend())))
				{
					this->_write_to_front<const_lvalue_reference>(val);

					_output = std::move(this->begin());
				}

				else if (((left == this->rbegin()) || (left == this->end())) && ((right == this->end()) || (right == this->rbegin())))
				{
					this->_write_to_back<const_lvalue_reference>(val);

					_output = std::move(this->rbegin());
				}

				else _output = std::move(this->_insert<const_lvalue_reference>(std::move(left), std::move(right), val));

				return _output;
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		___constexpr20___ iterator insert(iterator left, iterator right, rvalue_reference val)
		{
			iterator _find_left = this->find(*left);
			iterator _find_right = this->find(*right);

			if((_find_left == this->end()) || (_find_right == this->end()))
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">->insert(iterator, iterator, rvalue_reference): Invalid iterators provided, no such values exist!"));
			}

			if ((left + 1) != right)
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">->insert(iterator, iterator, rvalue_reference): Invalid iterators provided!"));
			}

			try
			{
				iterator _output = nullptr;

				if (((left == this->rend()) || (left == this->begin())) && ((right == this->begin()) || (right == this->rend())))
				{
					this->_write_to_front<rvalue_reference>(std::move(val));

					_output = std::move(this->begin());
				}

				else if (((left == this->rbegin()) || (left == this->end())) && ((right == this->end()) || (right == this->rbegin())))
				{
					this->_write_to_back<rvalue_reference>(std::move(val));

					_output = std::move(this->rbegin());
				}

				else _output = std::move(this->_insert<rvalue_reference>(std::move(left), std::move(right), std::move(val)));

				return _output;
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		template <typename... Args>
		___constexpr20___ void emplace_front(Args&&... constructor_args)
		{
			try
			{
				this->_write_to_front<rvalue_reference>(std::move(value_type(std::forward<Args>(constructor_args)...)));
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		template <typename... Args>
		___constexpr20___ void emplace_back(Args&&... constructor_args)
		{
			try
			{
				this->_write_to_back<rvalue_reference>(std::move(value_type(std::forward<Args>(constructor_args)...)));
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		template <typename... Args>
		___constexpr20___ iterator emplace(iterator left, iterator right, Args&&... constructor_args)
		{
			iterator _find_left = this->find(*left);
			iterator _find_right = this->find(*right);

			if ((_find_left == this->end()) || (_find_right == this->end()))
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">->emplace(iterator, iterator, Args&&...): Invalid iterators provided, no such values exist!"));
			}

			if ((left + 1) != right)
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">->emplace(iterator, iterator, Args&&...): Invalid iterators provided!"));
			}

			try
			{
				iterator _output = nullptr;

				if (((left == this->rend()) || (left == this->begin())) && ((right == this->begin()) || (right == this->rend())))
				{
					this->_write_to_front<rvalue_reference>(std::move(value_type(std::forward<Args>(constructor_args)...)));

					_output = std::move(this->begin());
				}

				else if (((left == this->rbegin()) || (left == this->end())) && ((right == this->end()) || (right == this->rbegin())))
				{
					this->_write_to_back<rvalue_reference>(std::move(value_type(std::forward<Args>(constructor_args)...)));

					_output = std::move(this->rbegin());
				}

				else _output = std::move(this->_insert<rvalue_reference>(std::move(left), std::move(right), std::move(value_type(std::forward<Args>(constructor_args)...))));

				return _output;
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		___constexpr20___ void prepend_range(std::initializer_list<value_type>&& range)
		{
			try
			{
				for (auto _iterator = (range.end() - 1); _iterator != (range.begin() - 1); --_iterator)
					this->_write_to_front<const_lvalue_reference>(*_iterator);
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		template <typename range_type>
		___constexpr20___ void prepend_range(range_type range)
		{
			if (std::is_lvalue_reference_v<range_type>)
			{
				try
				{
					if (typeid(range_type) != typeid(sequence<value_type>))
					{
						for (auto _iterator = range.rbegin(); _iterator != range.rend(); ++_iterator)
							this->_write_to_front<const_lvalue_reference>(*_iterator);
					}

					else
					{
						for (auto _iterator = range.rbegin(); _iterator != range.rend(); --_iterator)
							this->_write_to_front<const_lvalue_reference>(*_iterator);
					}
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}

			else
			{
				try
				{
					if (typeid(range_type) != typeid(sequence<value_type>))
					{
						for (auto _iterator = range.rbegin(); _iterator != range.rend(); ++_iterator)
							this->_write_to_front<rvalue_reference>(std::move(*_iterator));
					}

					else
					{
						for (auto _iterator = range.rbegin(); _iterator != range.rend(); --_iterator)
							this->_write_to_front<rvalue_reference>(std::move(*_iterator));
					}
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}
		}

		___constexpr20___ void append_range(std::initializer_list<value_type>&& range)
		{
			try
			{
				for (auto _iterator = range.begin(); _iterator != range.end(); ++_iterator)
					this->_write_to_back<const_lvalue_reference>(*_iterator);
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		template <typename range_type>
		___constexpr20___ void append_range(range_type range)
		{
			if (std::is_lvalue_reference_v<range_type>)
			{
				try
				{
					for (auto _iterator = range.begin(); _iterator != range.end(); ++_iterator)
						this->_write_to_back<const_lvalue_reference>(*_iterator);
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}

			else
			{
				try
				{
					for (auto _iterator = range.begin(); _iterator != range.end(); ++_iterator)
						this->_write_to_back<rvalue_reference>(std::move(*_iterator));
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}
		}

		___constexpr20___ void pop_front()
		{
			try
			{
				this->_free_from_front();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		___constexpr20___ void pop_back()
		{
			try
			{
				this->_free_from_back();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		___constexpr20___ void erase(iterator pos)
		{
			iterator _find_out = this->find(*pos);

			if (_find_out == this->end())
			{
				throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
					std::string(">->erase(iterator)->find(const value_type): No such value exists!"));
			}

			try
			{
				if (pos == this->begin())
					this->_free_from_front();

				else if (pos == this->rbegin())
					this->_free_from_back();

				else this->_erase(std::move(pos));
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		___constexpr20___ void clear()
		{
			if (!this->empty())
			{
				try
				{
					this->_free_cluster_unit();
				}

				catch (sequence_error _error)
				{
					throw;
				}
			}
		}

		___nodiscard___ ___constexpr20___ iterator find(const value_type val) const
		{
			iterator _output = this->begin();

			while (_output != this->end())
			{
				if (*_output == val) break;

				++_output;
			}

			return _output;
		}

		___nodiscard___ ___constexpr20___ iterator begin() const
		{
			return iterator(_core._front_module, _core._front);
		}

		___nodiscard___ ___constexpr20___ iterator end() const
		{
			return iterator(_core._back_module, (_core._back + 1));
		}

		___nodiscard___ ___constexpr20___ iterator rbegin() const
		{
			return iterator(_core._back_module, _core._back);
		}

		___nodiscard___ ___constexpr20___ iterator rend() const
		{
			return iterator(_core._front_module, (_core._front - 1));
		}

		___nodiscard___ ___constexpr20___ const size_type size() const
		{
			size_type _counter = 0;

			if (_core._front_module == _core._back_module)
			{
				_counter = static_cast<size_type>((_core._back + 1) - _core._front);
			}

			else if ((_core._front_module->_successor == _core._back_module) && (_core._back_module->_antecessor == _core._front_module))
			{
				_counter = (static_cast<size_type>(_core._front_module->_end - _core._front) + static_cast<size_type>((_core._back + 1) - _core._back_module->_begin));
			}

			else
			{
				_counter = (static_cast<size_type>(_core._front_module->_end - _core._front) + static_cast<size_type>((_core._back + 1) - _core._back_module->_begin));

				module_pointer _iterator = nullptr;

				if (_core._front_module->_successor != nullptr)
					_iterator = _core._front_module->_successor;

				else
				{
					throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
						std::string(">->capacity(): Successor pointer was null, could not fully calculate size!"));
				}

				while (_iterator != _core._back_module)
				{
					_counter += static_cast<size_type>(_iterator->_end - _iterator->_begin);

					if (_iterator->_successor != nullptr)
						_iterator = _iterator->_successor;

					else
					{
						throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
							std::string(">->capacity(): Successor pointer was null, could not fully calculate size!"));
					}
				}
			}

			const size_type _size = _counter;

			return _size;
		}

		___nodiscard___ ___constexpr20___ const size_type capacity() const
		{
			size_type _counter = 0;

			module_pointer _iterator = _core._front_module;

			while (_iterator != _core._back_module)
			{
				_counter += static_cast<size_type>(_iterator->_end - _iterator->_begin);

				if (_iterator->_successor != nullptr)
					_iterator = _iterator->_successor;

				else
				{
					throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
						std::string(">->capacity(): Successor pointer was null, could not fully calculate capacity!"));
				}
			}

			_counter += static_cast<size_type>(_iterator->_end - _iterator->_begin);

			const size_type _capacity = _counter;

			return _capacity;
		}

		___nodiscard___ ___constexpr20___ const size_type module_count() const
		{
			size_type _counter = 0;

			module_pointer _iterator = _core._front_module;

			while (_iterator != _core._back_module)
			{
				++_counter;

				if (_iterator->_successor != nullptr)
					_iterator = _iterator->_successor;

				else
				{
					throw sequence_error(std::string("dt0::sequence<") + std::string(typeid(value_type).name()) +
						std::string(">->module_count(): Successor pointer was null, could not fully calculate capacity!"));
				}
			}

			_counter += 1;

			const size_type _module_count = _counter;

			return _module_count;
		}

		___nodiscard___ ___constexpr20___ const size_type module_size() const
		{
			return (sizeof(module_type) + (_heap_module_reserve_capacity * sizeof(value_type)));
		}

		___nodiscard___ ___constexpr20___ const_lvalue_reference front() const
		{
			return *_core._front;
		}

		___nodiscard___ ___constexpr20___ const_lvalue_reference back() const
		{
			return *_core._back;
		}

		___nodiscard___ ___constexpr20___ lvalue_reference front()
		{
			return *_core._front;
		}

		___nodiscard___ ___constexpr20___ lvalue_reference back()
		{
			return *_core._back;
		}

		___nodiscard___ ___constexpr20___ bool empty() const
		{
			if ((_core._front == nullptr) && (_core._back == nullptr))
				return true;

			return false;
		}

		___constexpr20___ void shrink_to_fit()
		{
			try
			{
				this->_compress();
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		void resize(const size_type new_size)
		{
			try
			{
				while (this->size() != new_size)
				{
					this->_free_from_back();
				}
			}

			catch (sequence_error _error)
			{
				throw;
			}
		}

		___nodiscard___ ___constexpr20___ allocator_type get_allocator() const
		{
			return allocator_type();
		}

		___nodiscard___ ___constexpr20___ size_type reserve_capacity() const
		{
			return _heap_module_reserve_capacity;
		}

	private:
		/* Sequence core values class */
		class sequence_core
		{
		public:
			/* Sequence core variables, they are responsible for insertion and deletion of elements,
			and also keep track of the bounds of the container */
			value_pointer  _front;
			value_pointer  _back;
			module_pointer _front_module;
			module_pointer _back_module;

			sequence_core() noexcept : _front(nullptr), _back(nullptr), _front_module(nullptr), _back_module(nullptr) {}

			~sequence_core() noexcept = default;
		};

		using base1 = typename inheritance_pair<allocator_type, sequence_core>::base1;	// Sequence allocator
		using base2 = typename inheritance_pair<allocator_type, sequence_core>::base2;  // Sequence core values class

		/* Cluster unit core */
		inheritance_pair<allocator_type, sequence_core> _core;

		/* The capacity of the reserved data space of each heap module 
		(excludes modules that are created or reallocated on insertion) */
		size_type _heap_module_reserve_capacity = 4ui64;
	};
}

#endif /* SEQUENCE_HPP */
