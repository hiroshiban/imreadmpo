% compile_mex.m
%
% This is a script for compiling the "decode_jpeg_mex.cpp" and
% generating the mex file for imreadmpo_mex.
%
% [setup for compiling]
% Here, I will leave notes for compiling the decode_jpeg_mex.cpp with
% Microsoft Visual Studio Community 2022 and MATLAB R2023a on Windows.
% While the compilation has been only tested on Windows, it will also
% work on MacOS without any issue if you have proper libjpg library
% installed on your machine.
%
% Please replace some lines below depending on your purposes and environments.
%
% [step 01: preparing libjpeg library to MS Visual Studio 2022 Community on Windows]
%
% (Please follow the steps below on the Windows PowerShell console)
%
% # move to the directory where you want to install vcpkg (an package installer)
% > cd C:\usr\local
%
% # download vcpkg repository
% > git clone https://github.com/microsoft/vcpkg
%
% # build vcpkg and generate the executable, vcpkg.exe
% > .\vcpkg\bootstrap-vcpkg.bat
%
% # installing libjpeg-turbo 64bit
% > .\vcpkg\vcpkg.exe install libjpeg-turbo:x64-windows
%
% [step 02: compiling decode_jpeg_mex.cpp file and generating the mex file]
%
% If the libjpeg library is installed properly, following the procedures above,
% the required files are stored in these directories below (please change
% these paths if you customize the install location of the library).
%
% path to include: C:\usr\local\vcpkg\installed\x64-windows\include
% path to library: C:\usr\local\vcpkg\installed\x64-windows\lib
%
% After setting these paths to the script below, please run mex and compile the file.
%
%
% Created    : "2025-10-03 16:31:18 ban"
% Last Update: "2025-10-03 16:39:40 ban"

% 1. confirming whether the Visual Studio 2022 Community is set as the current MATLAB C++ compiler
% if not, please select "Microsoft Visual C++ 2022" from the list
mex -setup C++

% 2. setting the paths of libjpeg installed via vcpkg
% please change vcpkg_root etc if you change the install location.
vcpkg_root   = 'C:\usr\local\vcpkg';
include_path = fullfile(vcpkg_root,'installed','x64-windows','include');
lib_path     = fullfile(vcpkg_root,'installed','x64-windows','lib');

% 3. checking whether the paths exist
if ~isfolder(include_path) || ~isfolder(lib_path)
  error('include or lib folder not found in the path, please check vcpkg installation location and retry.');
end

% 4. run mex compilation
% with setting the include path by -I and the library path by -L options
disp('compiling mex files...');
try
  mex(['-I"' include_path '"'], ['-L"' lib_path '"'], 'decode_jpeg_mex.cpp', '-ljpeg');
  disp('The MEX file was compiled successfully.');
catch ME
  disp('failed to compile the MEX file. check the error message.');
  rethrow(ME);
end
