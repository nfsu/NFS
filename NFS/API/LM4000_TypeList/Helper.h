#pragma once
#include <string>

namespace lag
{
	struct helper
	{
		static std::string removeBullshit(std::string str, bool removeTemplate = false)
		{
			unsigned int start = 0;
			size_t size = str.size();
			for (unsigned int i = 0; i < str.size(); ++i)
			{
				if (str[i] == ' ')
				{
					start = i + 1;
				}
				if (str[i] == ':')
				{
					start = i + 1;
				}
				if (removeTemplate)
				{
					if (str[i] == '<')
					{
						size = i;
					}
				}
			}
			return std::string(&str[start], size - start);
		}
	};
}