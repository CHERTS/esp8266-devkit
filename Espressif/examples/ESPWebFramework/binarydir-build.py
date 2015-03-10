from distutils.core import setup
import py2exe
     
setup(
    name='binarydir',
    version='0.1.0',
    license='GPLv2+',
    author='Fabrizio Di Vittorio',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Embedded Systems',
        'Environment :: Console',
        'License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)',
        'Programming Language :: Python :: 2.7',
    ],
    console=[{'script': 'binarydir.py'}],
    options={'py2exe': {'includes':['slimmer','mimetypes','struct','sys','glob','pkgutil'],'optimize':2}}
)
