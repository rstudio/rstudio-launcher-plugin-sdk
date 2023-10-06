#!dgroovy

//
// JenkinsFile
//
// Copyright (C) 2019-20 by RStudio, PBC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//


properties([
    disableConcurrentBuilds(),
    buildDiscarder(logRotator(artifactDaysToKeepStr: '',
                              artifactNumToKeepStr: '',
                              daysToKeepStr: '',
                              numToKeepStr: '100')),
    parameters([string(name: 'RLP_SDK_VERSION_MAJOR', defaultValue: '1', description: 'RStudio Launcher Plugin SDK Major Version'),
                string(name: 'RLP_SDK_VERSION_MINOR', defaultValue: '0', description: 'RStudio Launcher Plugin SDK Minor Version'),
                string(name: 'SLACK_CHANNEL', defaultValue: '#ide-builds', description: 'Slack channel to publish build message.'),
                booleanParam(name: 'UPLOAD_PACKAGE', description: 'When checked, pushes the generated package to AWS S3.')
                ])
])

def create_package() {
  // start with major, minor, and patch versions
  def env = "RLP_SDK_VERSION_MAJOR=${rlpSdkVersionMajor} RLP_SDK_VERSION_MINOR=${rlpSdkVersionMinor} RLP_SDK_VERSION_PATCH=${rlpSdkVersionPatch}"

  // build the package
  sh "${env} docker/package/make-package.sh"
}

def generate_documentation() {
  sh "tools/generate-documentation.sh '${rlpSdkVersionMajor}.${rlpSdkVersionMinor}.${rlpSdkVersionPatch}'"
}

def build_source(type) {
  // start with major, minor, and patch versions
  def env = "RLP_SDK_VERSION_MAJOR=${rlpSdkVersionMajor} RLP_SDK_VERSION_MINOR=${rlpSdkVersionMinor} RLP_SDK_VERSION_PATCH=${rlpSdkVersionPatch}"

  // currently our nodes have access to 4 cores, so spread out the compile job
  // a little (currently using up all 4 cores causes problems)
  env = "${env} MAKEFLAGS=-j3 CMAKE_BUILD_TYPE=${type}"

  sh "${env} docker/jenkins/compile.sh"
}

def run_tests(type) {
  // attempt to run tests
  sh "cd .."
  sh "tools/run-all-tests.sh cmake-build-${type}"
}

def s3_upload() {
  def version = "${rlpSdkVersionMajor}.${rlpSdkVersionMinor}.${rlpSdkVersionPatch}"
  def buildFolder = "docker/package"
  def packageFile = sh (
      script: "basename `ls ${buildFolder}/*-${version}.tar.gz`",
      returnStdout: true
  ).trim()

  // copy installer to s3
  sh "aws s3 cp ${buildFolder}/${packageFile} s3://rstudio-launcher-plugin-sdk/"
  
  // copy documentation to s3
  sh "aws s3 sync docs/ApiRefHtml s3://docs.rstudio.com/rlps/apiref/${version}/"
  sh "aws s3 sync docs/ApiRefHtml s3://docs.rstudio.com/rlps/apiref/latest/"
    
  sh "aws s3 sync docs/QuickStartHtml s3://docs.rstudio.com/rlps/quickstart/${version}/"
  sh "aws s3 sync docs/QuickStartHtml s3://docs.rstudio.com/rlps/quickstart/latest/"
    
  sh "aws s3 sync docs/DevGuideHtml s3://docs.rstudio.com/rlps/devguide/${version}/"
  sh "aws s3 sync docs/DevGuideHtml s3://docs.rstudio.com/rlps/devguide/latest/"
}

def jenkins_user_build_args() {
  def jenkins_uid = sh (script: 'id -u jenkins', returnStdout: true).trim()
  def jenkins_gid = sh (script: 'id -g jenkins', returnStdout: true).trim()
  return " --build-arg JENKINS_UID=${jenkins_uid} --build-arg JENKINS_GID=${jenkins_gid}"
}

def prepareWorkspace() { // accessory to clean workspace and checkout
  step([$class: 'WsCleanup'])
  checkout scm
  sh 'git reset --hard && git clean -ffdx' // lifted from rstudio/connect
}
// forward declare version vars
rlpSdkVersionMajor  = 0
rlpSdkVersionMinor  = 0
rlpSdkVersionPatch  = 0

// make a nicer slack message
messagePrefix = "Jenkins ${env.JOB_NAME} build: <${env.BUILD_URL}display/redirect|${env.BUILD_DISPLAY_NAME}>"

try {
  timestamps {
    def containers = [
      [os: 'bionic',    arch: 'amd64',    flavor: 'Release'],
      [os: 'bionic',    arch: 'amd64',    flavor: 'Debug'],
      [os: 'centos8',   arch: 'x86_64',   flavor: 'Release']
    ]

    // create the version we're about to build
    node('docker') {
      stage('Prepare Versioning Container') {
        prepareWorkspace()
        container = pullBuildPush(image_name: 'jenkins/rlp-sdk', dockerfile: "docker/jenkins/Dockerfile.versioning", image_tag: "rlp-sdk-versioning", build_args: jenkins_user_build_args())
        container.inside() {
          stage('Bump Version') {
            def rlpSdkVersion = sh (
              script: "docker/jenkins/rlps-version.sh bump ${params.RLP_SDK_VERSION_MAJOR}.${params.RLP_SDK_VERSION_MINOR}",
              returnStdout: true
            ).trim()
            echo "RStudio Launcher Plugin SDK build version: ${rlpSdkVersion}"
            def components = rlpSdkVersion.split('\\.')

            // extract version
            rlpSdkVersionMajor = components[0]
            rlpSdkVersionMinor = components[1]
            rlpSdkVersionPatch = components[2]

            // update slack message to include build version
            messagePrefix = "Jenkins ${env.JOB_NAME} build: <${env.BUILD_URL}display/redirect|${env.BUILD_DISPLAY_NAME}>, version: ${rlpSdkVersion}"
          }
        }
      }
    }

    // build each variant in parallel
    def parallel_containers = [:]
    for (int i = 0; i < containers.size(); i++) {
      def index = i
      parallel_containers["${containers[i].os}-${containers[i].arch}-${containers[i].flavor}"] = {
        def current_container = containers[index]
        def container

        node('docker') {
          stage('Prepare B&T Container') {
            prepareWorkspace()
            def image_tag = "${current_container.os}-${current_container.arch}-${params.RLP_SDK_VERSION_MAJOR}.${params.RLP_SDK_VERSION_MINOR}"
            withCredentials([usernameColonPassword(credentialsId: 'posit-jenkins', variable: "github_login")]) {
              def github_args = '--build-arg GITHUB_LOGIN=${github_login}'
              container = pullBuildPush(image_name: 'jenkins/rlp-sdk', dockerfile: "docker/jenkins/Dockerfile.${current_container.os}-${current_container.arch}", image_tag: image_tag, build_args: github_args + " " + jenkins_user_build_args())
            }
          }
          stage('Build and Test') {
            container.inside("--privileged") {
              stage('Compile Source') {
                build_source("${current_container.flavor}")
              }
              stage('Run Tests') {
                run_tests("${current_container.flavor}")
              }
            }
          }
        }
      }
    }

    parallel parallel_containers

    stage ('Package and Upload SDK') {
      node ('docker') {
        stage('Prepare Packaging Container') {
          prepareWorkspace()
          container = pullBuildPush(image_name: 'jenkins/rlp-sdk', dockerfile: "docker/jenkins/Dockerfile.packaging", image_tag: "rlp-sdk-packaging", build_args: jenkins_user_build_args())
          container.inside() {
            stage('Generate Documentation') {
              generate_documentation()
            }
            stage('Create Package') {
              create_package()
            }
          }
          if (params.get('UPLOAD_PACKAGE') == true) {
            stage('Upload Package') {
              s3_upload()
            }
          }
        }
      }
    }

    slackSend channel: params.get('SLACK_CHANNEL', '#ide-builds'), color: 'good', message: "${messagePrefix} passed"
  }
} catch(err) {
   slackSend channel: params.get('SLACK_CHANNEL', '#ide-builds'), color: 'bad', message: "${messagePrefix} failed: ${err}"
   error("failed: ${err}")
}
