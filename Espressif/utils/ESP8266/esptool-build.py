from distutils.core import setup
import py2exe
import io
import os
import re

# Example code to pull version from esptool.py with regex, taken from
# http://python-packaging-user-guide.readthedocs.org/en/latest/single_source_version/
def read(*names, **kwargs):
    with io.open(
            os.path.join(os.path.dirname(__file__), *names),
            encoding=kwargs.get("encoding", "utf8")
    ) as fp:
        return fp.read()


def find_version(*file_paths):
    version_file = read(*file_paths)
    version_match = re.search(r"^__version__ = ['\"]([^'\"]*)['\"]",
                              version_file, re.M)
    if version_match:
        return version_match.group(1)
    raise RuntimeError("Unable to find version string.")
     
setup(
    name='esptool',
    description='A utility to communicate with the ROM bootloader in Espressif ESP8266.',
    version=find_version('esptool.py'),
    license='GPLv2+',
    author='Fredrik Ahlberg (themadinventor) & Angus Gratton (projectgus)',
    author_email='angus@espressif.com',
    url='https://github.com/espressif/esptool',
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'Natural Language :: English',
        'Operating System :: POSIX',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: MacOS :: MacOS X',
        'Topic :: Software Development :: Embedded Systems',
        'Environment :: Console',
        'License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
    ],
    console=[{'script': 'esptool.py'}],
    options={'py2exe': {'includes':['binascii','re','os','re','string','sys','serial','argparse','json','hashlib','inspect','base64','zlib','shlex','time','struct'],'optimize':2}}
)
