import logging
logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
logger.addHandler(logging.StreamHandler())

class MyClass:
	def _log(f):
		def wrapper(self, *args, **kwargs):
			value = f(self, *args, **kwargs)
			logger.debug('%s :: args: %s, kwargs: %s',
				f.__name__, args, kwargs)
			return value
		return wrapper


	@_log
	def sum(self, a, b):
		return a + b


c = MyClass()
print c.sum(2, 3)