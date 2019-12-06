#!groovy

//
// JenkinsFile
//
// Copyright (C) 2019 by RStudio Launcher Plugin SDK, Inc.
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
    parameters([string(name: 'RLP_SDK_VERSION_MAJOR', defaultValue: '0', description: 'RStudio Launcher Plugin SDK Major Version'),
                string(name: 'RLP_SDK_VERSION_MINOR', defaultValue: '1', description: 'RStudio Launcher Plugin SDK Minor Version'),
                string(name: 'SLACK_CHANNEL', defaultValue: '#ide-builds', description: 'Slack channel to publish build message.'),
                booleanParam(name: 'CREATE_PACKAGE', description: 'When checked, creates a release package and pushes it to AWS S3.')
                ])
])

def create_package() {
  // start with major, minor, and patch versions
  def env = "RLP_SDK_VERSION_MAJOR=${rlpSdkVersionMajor} RLP_SDK_VERSION_MINOR=${rlpSdkVersionMinor} RLP_SDK_VERSION_PATCH=${rlpSdkVersionPatch}"

  // build the package
  sh "${env} package/make-package.sh"
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
  try {
    // attempt to run unit tests
    sh "cd .."
    sh "docker/jenkins/run-all-tests.sh cmake-build-${type}"
  } catch(err) {
    // mark build as unstable if it fails unit tests
    currentBuild.result = "UNSTABLE"
  }
}

def s3_upload() {
  def buildFolder = "docker/package"
  def packageFile = sh (
      script: "basename `ls ${buildFolder}/*-${rlpSdkVersionMajor}.${rlpSdkVersionMinor}.${rlpSdkVersionPatch}.tar.gz`",
      returnStdout: true
  ).trim()

  // copy installer to s3
  sh "aws s3 cp ${buildFolder}/${packageFile} s3://rstudio-launcher-plugin-sdk/"
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
      stage('set up versioning') {
        prepareWorkspace()
        container = pullBuildPush(image_name: 'jenkins/ide', dockerfile: "docker/jenkins/Dockerfile.versioning", image_tag: "rlp-sdk-versioning", build_args: jenkins_user_build_args())
        container.inside() {
          stage('bump version') {
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

        node('ide') {
          stage('prepare ws/container') {
            prepareWorkspace()
            def image_tag = "${current_container.os}-${current_container.arch}-${params.RLP_SDK_VERSION_MAJOR}.${params.RLP_SDK_VERSION_MINOR}"
            withCredentials([usernameColonPassword(credentialsId: 'github-rstudio-jenkins', variable: "github_login")]) {
              def github_args = "--build-arg GITHUB_LOGIN=${github_login}"
              container = pullBuildPush(image_name: 'jenkins/rlpSdk', dockerfile: "docker/jenkins/Dockerfile.${current_container.os}-${current_container.arch}", image_tag: image_tag, build_args: github_args + " " + jenkins_user_build_args())
            }
          }
          stage('Build and Test') {
            container.inside() {
              stage('compile source') {
                build(current_container.flavor)
              }
              stage('run tests') {
                run_tests(current_container.flavor)
              }
            }
          }
        }
      }
    }

    parallel parallel_containers

    stage ('package and upload SDK') {
      when {
        expression { return params.CREATE_PACKAGE }
      }
      node ('sdk package') {
        stage('set up packaging') {
          prepareWorkspace()
          container = pullBuildPush(image_name: 'jenkins/rlpSdk', dockerfile: "docker/jenkins/Dockerfile.packaging", image_tag: "rlp-sdk-packaging", build_args: jenkins_user_build_args())
          container.inside() {
            stage('create package') {
              create_package()
            }
          }
          stage('upload package') {
            s3_upload()
          }
        }
      }
    }

    slackSend channel: params.get('SLACK_CHANNEL', '#ide-builds'), color: 'good', message: "${messagePrefix} passed (${currentBuild.result})"
  }
} catch(err) {
   slackSend channel: params.get('SLACK_CHANNEL', '#ide-builds'), color: 'bad', message: "${messagePrefix} failed: ${err}"
   error("failed: ${err}")
}
