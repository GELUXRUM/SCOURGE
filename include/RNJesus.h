#pragma once

class RNJesus
{
public:
	RNJesus() :
		generator(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count())) {}

	template <typename T>
	void CheckMinMax(T& a_min, T& a_max)
	{
		if (a_min > a_max) {
			float temp = a_max;
			a_max = a_min;
			a_min = temp;
		}
	}

	template <typename T>
	T GaussianDistribution(T a_minValue, T a_maxValue)
	{
		T mean = (a_maxValue + a_minValue) / T(2.0);  // stay safe kids
		T stddev = (a_maxValue - mean) / T(3.33333333333333333333333333333);

		std::normal_distribution<T> distribution(mean, stddev);

		T number = distribution(generator);
		
		if (number < a_minValue) {
			number = a_minValue;
		} else if (number > a_maxValue) {
			number = a_maxValue;
		}

		return number;
	}

	bool GetRandomBool()
	{
		std::bernoulli_distribution distribution(0.5f);
		return distribution(generator);
	}

	template <typename T>
	T BiasedGaussianDistribution(T a_minValue, T a_maxValue, T a_meanMult)  // a_mean must be between 0.0 and 1.0
	{
		bool useRightTail = GetRandomBool();

		T mean = a_minValue + ((a_maxValue - a_minValue) * a_meanMult);
		T stddev;
		if (useRightTail) {
			stddev = (a_maxValue - mean) / T(3.33333333333333333333333333333);
		} else {
			stddev = (mean - a_minValue) / T(3.33333333333333333333333333333);
		}

		std::normal_distribution<T> distribution(mean, stddev);
		T number;
		do {
			number = distribution(generator);
		} while ((useRightTail && number < mean) || (!useRightTail && number > mean));

		if (number < a_minValue) {
			number = a_minValue;
		} else if (number > a_maxValue) {
			number = a_maxValue;
		}

		return number;
	}

	template <typename T>
	T DoGaussian(T a_minValue, T a_maxValue, T a_meanMult)
	{
		CheckMinMax(a_minValue, a_maxValue);

		if (a_meanMult == T(0.5)) {
			return GaussianDistribution(a_minValue, a_maxValue);
		} else if (a_meanMult == T(0.0)) {
			T result;
			T newMin = a_minValue - (a_maxValue - a_minValue);
			do {
				result = GaussianDistribution(newMin, a_maxValue);
			} while (result < a_minValue);
			return result;
		} else if (a_meanMult == T(1.0)) {
			T result;
			T newMax = a_maxValue + (a_maxValue - a_minValue);
			do {
				result = GaussianDistribution(a_minValue, newMax);
			} while (result > a_maxValue);
			return result;
		} else {
			return BiasedGaussianDistribution(a_minValue, a_maxValue, a_meanMult);
		}
	}

public:
	std::mt19937 generator;
};
