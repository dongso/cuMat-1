#ifndef __CUMAT_DEVICE_POINTER_H__
#define __CUMAT_DEVICE_POINTER_H__

#include <cuda_runtime.h>

#include "Macros.h"
#include "Context.h"

CUMAT_NAMESPACE_BEGIN

template <typename T>
class DevicePointer
{
private:
	T* pointer_;
	size_t* counter_;
	cuMat::Context* context_;

    __host__ __device__
	void release()
	{
#ifndef __CUDA_ARCH__
        //no decrement of the counter in CUDA-code, counter is in host-memory
		if ((counter_) && (--(*counter_) == 0))
		{
			delete counter_;
			context_->freeDevice(pointer_);
		}
#endif
	}

public:
	DevicePointer(size_t size) 
		: pointer_(nullptr)
		, counter_(nullptr)
	{
		context_ = &cuMat::Context::current();
		pointer_ = static_cast<T*>(context_->mallocDevice(size * sizeof(T)));
		try {
			counter_ = new size_t(1);
		}
		catch (...)
		{
			context_->freeDevice(pointer_);
			throw;
		}
	}

    __host__ __device__
	DevicePointer()
		: pointer_(nullptr)
		, counter_(nullptr)
		, context_(nullptr)
	{}

    __host__ __device__
	DevicePointer(const DevicePointer<T>& rhs)
		: pointer_(rhs.pointer_)
		, counter_(rhs.counter_)
		, context_(rhs.context_)
	{
#ifndef __CUDA_ARCH__
        //no increment of the counter in CUDA-code, counter is in host-memory
		if (counter_) {
			++(*counter_);
		}
#endif
	}

    __host__ __device__
	DevicePointer(DevicePointer<T>&& rhs) noexcept
		: pointer_(std::move(rhs.pointer_))
		, counter_(std::move(rhs.counter_))
		, context_(std::move(rhs.context_))
	{}

    __host__ __device__
	DevicePointer<T>& operator=(const DevicePointer<T>& rhs)
	{
		release();
		pointer_ = rhs.pointer_;
		counter_ = rhs.counter_;
		context_ = rhs.context_;
#ifndef __CUDA_ARCH__
        //no increment of the counter in CUDA-code, counter is in host-memory
		if (counter_) {
			++(*counter_);
		}
#endif
		return *this;
	}

    __host__ __device__
	DevicePointer<T>& operator=(DevicePointer<T>&& rhs) noexcept
	{
		release();
		pointer_ = std::move(rhs.pointer_);
		counter_ = std::move(rhs.counter_);
		context_ = std::move(rhs.context_);
		return *this;
	}

    __host__ __device__
	void swap(DevicePointer<T>& rhs) throw()
	{
		std::swap(pointer_, rhs.pointer_);
		std::swap(counter_, rhs.counter_);
		std::swap(context_, rhs.context_);
	}

    __host__ __device__
	~DevicePointer()
	{
		release();
	}

	__host__ __device__ T* pointer() { return pointer_; }
	__host__ __device__ const T* pointer() const { return pointer_; }

	/**
	 * \brief Returns the current value of the reference counter.
	 * This can be used to determine if this memory is used uniquely
	 * by an object.
	 * \return the current number of references
	 */
	size_t getCounter() const { return counter_ ? *counter_ : 0; }
};

CUMAT_NAMESPACE_END

#endif