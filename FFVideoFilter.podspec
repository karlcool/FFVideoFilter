Pod::Spec.new do |s|
  s.name         = "FFVideoFilter"
  s.version      = "1.0.0"
  s.summary      = "FFmpeg视频实时滤镜"
  s.homepage     = "https://github.com/karlcool/FFVideoFilter"
  s.license      = 'GPL'
  s.platform     = :ios
  s.requires_arc = true
  s.author       = { "karlcool.l" => "karlcool.l@qq.com" }
  s.source       = { :git => "https://github.com/karlcool/FFVideoFilter.git", :tag => "#{s.version}" }
  s.source_files  = 'Class/**/*.{h,m,cpp}'
  s.ios.deployment_target = '9.0'
  s.frameworks = 'AVFoundation', 'Accelerate', 'AudioToolbox', 'CoreAudio', 'CoreGraphics', 'CoreMedia', 'MediaPlayer', 'QuartzCore', 'VideoToolbox'
  s.libraries = 'iconv', 'bz2', 'z'
  s.vendored_frameworks = 'FFmpeg.framework'

$dir = File.dirname(__FILE__)
$dir = $dir + "Class/kit/**" 
s.pod_target_xcconfig = { 'HEADER_SEARCH_PATHS' => $dir}
end